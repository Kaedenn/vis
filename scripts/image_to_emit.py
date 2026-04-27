#!/usr/bin/env python3

"""
Convert image(s) to Lua emit calls
"""

# TODO: Implement a sort of "defaults" and "overrides" tables for the generated
# emits (ideally in an efficient way) so that broad-reaching changes can be
# made without regenerating each Emit frame. This should work with both Emit()
# and Vis.emit().

# TODO: Implement a form of temporal compression to reduce Emit calls: the
# lifetime of a particle should be increased if the pixel is unchanged on the
# subsequent frame. Instead of generating two emits, generate one with the
# frame duration added to the lifetime.

import argparse
from contextlib import contextmanager
import enum
import logging
import math
import os
import sys
import textwrap

import numpy as np
from PIL import Image

TRACE = 5 # DEBUG=10, INFO=20
class TraceLogger(logging.Logger):
  "Custom trace logger"
  def trace(self, *args, **kwargs):
    "Report a trace message"
    return self.log(TRACE, *args, **kwargs)
logging.addLevelName(TRACE, "TRACE")
logging.setLoggerClass(TraceLogger)

logging.basicConfig(format="%(module)s:%(lineno)s: %(levelname)s: %(message)s",
                    level=logging.INFO)
logger = logging.getLogger(__name__)

EMIT_FIELDS = ("count", "ux", "uy", "depth", "s", "us", "ds", "uds",
      "rad", "urad", "theta", "utheta", "life", "ulife", "ur", "ug", "ub",
      "force", "limit", "blender", "tag")

GEN_SETTINGS_COMMENT = "-- GENERATION SETTINGS: "
GEN_SETTINGS_DELIM = ": "

class Optimizer(enum.Enum):
  "Method to use for reducing the number of emit calls"
  NONE = 0
  DESAMPLE2 = 1 << 0
  DESAMPLE4 = 1 << 1
  HSTRIP = 1 << 4
  HSTRIP2 = (1 << 4) + (1 << 0)
  HSTRIP4 = (1 << 4) + (1 << 1)
  VSTRIP = 1 << 8
  VSTRIP2 = (1 << 8) + (1 << 0)
  VSTRIP4 = (1 << 8) + (1 << 1)

class ThresholdFunc(enum.Enum):
  "Threshold function used to prune unneeded pixels from the output"
  NONE = enum.auto()
  LESS = enum.auto()
  MORE = enum.auto()

def optimizer_has(arg, base_value):
  "Determine if the optimizer includes the given base"
  value1 = arg.value if isinstance(arg, Optimizer) else arg
  value2 = base_value.value if isinstance(base_value, Optimizer) else base_value
  return (value1 & value2) != 0

def optimizer_arg_to_func(arg):
  "Convert a named optimizer function to a optimizer function instance"
  if hasattr(Optimizer, arg):
    arg_value = getattr(Optimizer, arg)
    if isinstance(arg_value, Optimizer):
      return arg_value
  raise ValueError(f"Invalid optimizer argument {arg}")

def threshold_arg_to_func(arg):
  "Convert a named threshold function to a threshold function instance"
  if hasattr(ThresholdFunc, arg):
    arg_value = getattr(ThresholdFunc, arg)
    if isinstance(arg_value, ThresholdFunc):
      return arg_value
  raise ValueError(f"Invalid threshold argument {arg}")

def evaluate_threshold(pixel, rule, argument):
  "Determine if we should emit this pixel"
  result = True
  if rule == ThresholdFunc.LESS:
    result = all(color > argument for color in pixel)
  elif rule == ThresholdFunc.MORE:
    result = all(color < argument for color in pixel)
  elif rule != ThresholdFunc.NONE:
    raise ValueError(f"Invalid threshold function {rule}")
  return result

# TODO: Incorporate motion data
def evaluate_optimizer(pixel_data, motion_data, func): # pylint: disable=unused-argument
  "Optimize iterating over the pixel and motion data"
  if func == Optimizer.NONE:
    for pixel_x, pixel_y in pixel_data:
      yield pixel_x, pixel_y, 0, 0

  elif optimizer_has(func, Optimizer.HSTRIP):
    array = pixels_to_ndarray(pixel_data)
    height, width, _ = array.shape
    off_axis_stride = 1
    if optimizer_has(func, Optimizer.DESAMPLE2):
      off_axis_stride = 2
    elif optimizer_has(func, Optimizer.DESAMPLE4):
      off_axis_stride = 4
    for curr_y in range(height):
      if optimizer_has(func, Optimizer.DESAMPLE2) and curr_y % 2 != 0:
        continue
      if optimizer_has(func, Optimizer.DESAMPLE4) and curr_y % 4 != 0:
        continue
      run_start_x = None
      run_pixel = None
      h_stride = 0
      for curr_x in range(width):
        pixel = array[curr_y, curr_x]
        if not np.any(pixel):
          # Empty pixel: flush any active run
          if run_pixel is not None:
            yield run_start_x, curr_y, h_stride, off_axis_stride
            run_start_x = None
            run_pixel = None
            h_stride = 0
          continue
        if run_pixel is None:
          run_start_x = curr_x
          run_pixel = pixel.copy()
          h_stride = 1
        elif np.array_equal(pixel, run_pixel):
          h_stride += 1
        else:
          # Different color: flush old run, start new one
          yield run_start_x, curr_y, h_stride, off_axis_stride
          run_start_x = curr_x
          run_pixel = pixel.copy()
          h_stride = 1
      # End of row: flush trailing run
      if run_pixel is not None:
        yield run_start_x, curr_y, h_stride, off_axis_stride

  elif optimizer_has(func, Optimizer.VSTRIP):
    array = pixels_to_ndarray(pixel_data)
    height, width, _ = array.shape
    off_axis_stride = 1
    if optimizer_has(func, Optimizer.DESAMPLE2):
      off_axis_stride = 2
    elif optimizer_has(func, Optimizer.DESAMPLE4):
      off_axis_stride = 4
    for curr_x in range(width):
      if optimizer_has(func, Optimizer.DESAMPLE2) and curr_x % 2 != 0:
        continue
      if optimizer_has(func, Optimizer.DESAMPLE4) and curr_x % 4 != 0:
        continue
      run_start_y = None
      run_pixel = None
      v_stride = 0
      for curr_y in range(height):
        pixel = array[curr_y, curr_x]
        if not np.any(pixel):
          # Empty pixel: flush any active run
          if run_pixel is not None:
            yield curr_x, run_start_y, off_axis_stride, v_stride
            run_start_y = None
            run_pixel = None
            v_stride = 0
          continue
        if run_pixel is None:
          run_start_y = curr_y
          run_pixel = pixel.copy()
          v_stride = 1
        elif np.array_equal(pixel, run_pixel):
          v_stride += 1
        else:
          # Different color: flush old run, start new one
          yield curr_x, run_start_y, off_axis_stride, v_stride
          run_start_y = curr_y
          run_pixel = pixel.copy()
          v_stride = 1
      # End of column: flush trailing run
      if run_pixel is not None:
        yield curr_x, run_start_y, off_axis_stride, v_stride

  elif func == Optimizer.DESAMPLE2:
    for pixel_x, pixel_y in pixel_data:
      if not (pixel_x % 2 or pixel_y % 2):
        yield pixel_x, pixel_y, 0, 0

  elif func == Optimizer.DESAMPLE4:
    for pixel_x, pixel_y in pixel_data:
      if not (pixel_x % 4 or pixel_y % 4):
        yield pixel_x, pixel_y, 0, 0

  else:
    raise ValueError(f"Invalid optimizer function {func}")

def parse_image_entry(entry):
  "Parse a single image entry of the form [NUM=]PATH[,PATH]"
  ftime, ppath, mpath = None, entry, None
  if "=" in entry:
    ftime, ppath = entry.split("=")
    if ftime[-1] == "f" and ftime[:-1].isdigit():
      ftime = f"Vis.frames2msec({ftime[:-1]})"
  if "," in ppath:
    ppath, mpath = ppath.split(",")
    if not os.path.isfile(mpath):
      logger.warning("Entry %s motion path %s is not a file", entry, mpath)
      mpath = None
  if not os.path.isfile(ppath):
    logger.warning("Entry %s pixel path %s is not a file", entry, ppath)
    return None, None, None
  return ppath, mpath, ftime

def gather_images(args):
  "Obtain a list of all images to process"
  results = {}
  if args.image_path:
    for image_path in args.image_path:
      ppath, mpath, ftime = parse_image_entry(image_path)
      if ppath is not None:
        results[ppath] = {"motion": mpath, "ftime": ftime}
  if args.file:
    with open(args.file, "rt", encoding="UTF-8") as fobj:
      for image_path in fobj.read().splitlines():
        ppath, mpath, ftime = parse_image_entry(image_path)
        if ppath is not None:
          results[ppath] = {"motion": mpath, "ftime": ftime}
  logger.debug("Read %d image record(s) from inputs: %s", len(results), results)
  return results

def extract_pixel_and_motion_data(image, motion, window_size=None, blank=None,
    thresholds=None, noscale=False):
  "Generate Lua commands for the given image (or pair) and write to the output"
  width, height = 0, 0
  pixel_data = {}
  motion_data = {}

  img = Image.open(image)
  if img.mode != "RGBA":
    img.convert("RGBA")
  if window_size is not None:
    img = resize_image_centered(img, *window_size, noscale=noscale)
  width, height = img.size
  for pidx, rgba in enumerate(img.get_flattened_data()):
    rgb = rgba[:3]
    pixel_x = pidx % width
    pixel_y = pidx // width % height
    if len(rgba) == 4 and rgba[3] == 0:
      continue
    if blank is not None:
      if rgb in blank:
        continue
    if thresholds:
      if not all(evaluate_threshold(rgb, func, arg) for func, arg in thresholds):
        continue
    pixel_data[(pixel_x, pixel_y)] = rgb
  img.close()

  if motion:
    img = Image.open(motion)
    if window_size is not None:
      img = resize_image_centered(img, *window_size, noscale=noscale)
    img = img.convert("HSV")
    if img.size != (width, height):
      logger.error("Motion image %s for %s has different size: %s != %s",
          motion, image, img.size, (width, height))
      raise ValueError("Color image and motion image must be the same size")

    for pidx, hsv in enumerate(img.get_flattened_data()):
      pixel_x = pidx % width
      pixel_y = pidx // width % height
      if (pixel_x, pixel_y) in pixel_data:
        motion_data[(pixel_x, pixel_y)] = hsv

  if pixel_data:
    logger.debug("Emitting %d of %d pixels (%02.02f%%)", len(pixel_data),
        width*height, len(pixel_data)/(width*height)*100)
    pixel_values = list(pixel_data.values())
    pixel_colors = set(pixel_values)
    total_pixels = sum(pixel_values.count(color) for color in pixel_colors)
    logger.debug("%d pixel colors appear an average of %d times each",
        len(pixel_colors), total_pixels // len(pixel_colors))

  return pixel_data, motion_data

def pixels_to_ndarray(pixel_data):
  "Build an ndarray to store the pixel data"
  coords = np.array(list(pixel_data.keys()))
  values = np.array(list(pixel_data.values()), dtype=np.uint8)
  xs = coords[:, 0]
  ys = coords[:, 1]
  width = xs.max() + 1
  height = ys.max() + 1
  stride = values.shape[1]
  array = np.zeros((height, width, stride), dtype=np.uint8)
  array[ys, xs] = values
  return array

def resize_image_centered(image, width, height, noscale=False):
  "Resize and center the given image, with a scaling toggle"
  # Ensure we have an alpha channel so transparency works correctly
  if image.mode != "RGBA":
    image = image.convert("RGBA")
  if width is None or height is None:
    return image
  if noscale:
    resized = image
    new_w, new_h = image.width, image.height
  else:
    # Compute scale factor to fit within target while preserving aspect ratio
    scale = min(width / image.width, height / image.height)
    new_w = int(round(image.width * scale))
    new_h = int(round(image.height * scale))
    # pylint: disable=no-member
    resized = image.resize((new_w, new_h), Image.NEAREST)
    # pylint: enable=no-member

  # Create transparent background
  result = Image.new("RGBA", (width, height), (0, 0, 0, 0))

  # Compute centered position (can be negative if oversized, but we scaled to fit)
  x = (width - new_w) // 2
  y = (height - new_h) // 2

  # Paste with alpha mask to preserve transparency
  result.paste(resized, (x, y), resized)

  return result

def flatten_strides(pixel_info, optimizer=Optimizer.NONE):
  """
  Merge consecutive identical entries to reduce Lua Emit() calls.

  pixel_info is a list of 4-tuples, all with units of pixels:
    base_x    between 0 ~ image.width
    base_y    between 0 ~ image.height
    width
    height

  For HSTRIP, we ignore y and height if x and width match
  For VSTRIP, we ignore x and width if y and height match
  """
  prior = []
  is_hstrip = optimizer_has(optimizer, Optimizer.HSTRIP)
  is_vstrip = optimizer_has(optimizer, Optimizer.VSTRIP)

  just_emitted = False
  for pixel_record in pixel_info:
    if optimizer == Optimizer.NONE:
      yield pixel_record
      just_emitted = True
      continue
    if not prior:
      prior = list(pixel_record)
      just_emitted = False
      continue

    pixelx, pixely, pixelw, pixelh = pixel_record
    priorx, priory, priorw, priorh = prior

    if is_hstrip:
      matches = pixelx == priorx and pixelw == priorw
    elif is_vstrip:
      matches = pixely == priory and pixelh == priorh
    if matches:
      logger.trace("Prior matches record: %s == %s via %s",
          prior, pixel_record, optimizer)
      just_emitted = False
      if is_hstrip:
        prior[3] += pixel_record[3]
      elif is_vstrip:
        prior[2] += pixel_record[2]
      logger.trace("Prior adjusted to %s with %s", prior, pixel_record)
    else:
      logger.trace("Emitting prior: %s (no match with %s via %s)",
          prior, pixel_record, optimizer)
      yield tuple(prior)
      prior = list(pixel_record)
      just_emitted = False    # We did not emit the current record
  if not just_emitted:
    logger.trace("Emitting last prior: %s", prior)
    yield tuple(prior)

# TODO: Change main for-loop to detect repeats in emit data across frames
def emit_lua(pixel_data, motion_data, ftime, output, optimizer=Optimizer.NONE,
    native=False, table_format="{{{table}}}", **overrides):
  "Build the final Lua code"
  num_emits = 0
  emit_fields = dict.fromkeys(EMIT_FIELDS, 0)
  emit_fields["rad"] = 1
  emit_fields["life"] = 10

  pixel_info = evaluate_optimizer(pixel_data, motion_data, optimizer)
  for pixel_x, pixel_y, stride_x, stride_y in \
      flatten_strides(pixel_info, optimizer=optimizer):
    emit_fields["count"] = 1
    pixel = pixel_data[(pixel_x, pixel_y)]
    motion_hsv = motion_data.get((pixel_x, pixel_y), (0, 0, 0))
    emit_fields["x"] = pixel_x
    emit_fields["y"] = pixel_y
    if stride_x > 0:
      emit_fields["x"] = pixel_x + stride_x // 2
      emit_fields["ux"] = stride_x / 2
      emit_fields["count"] = emit_fields["count"] * stride_x
    if stride_y > 0:
      emit_fields["y"] = pixel_y + stride_y // 2
      emit_fields["uy"] = stride_y / 2
      emit_fields["count"] = emit_fields["count"] * stride_y
    emit_fields["theta"] = motion_hsv[0] / 360 * 2 * math.pi
    emit_fields["ds"] = motion_hsv[1] / 255
    emit_fields["r"] = pixel[0] / 255
    emit_fields["g"] = pixel[1] / 255
    emit_fields["b"] = pixel[2] / 255

    emit_fields.update(**overrides)
    if native:
      count = get_and_remove(emit_fields, "count")
      table = ", ".join(f"{key}={value}" for key, value in emit_fields.items())
      lua_table_call = table_format.format(table=table)
      if ftime is not None:
        lua_code = f"Vis.emit(Vis.flist, {count}, {ftime}, {lua_table_call})"
      else:
        lua_code = f"Vis.emitnow(Vis.script, {count}, {lua_table_call})"
    else:
      table = ", ".join(f"{key}={value}" for key, value in emit_fields.items())
      lua_table_call = table_format.format(table=table)
      if ftime is not None:
        lua_code = f"Emit:new({lua_table_call}):emit_at({ftime})"
      else:
        lua_code = f"Emit:new({lua_table_call}):emit_now()"

    output.write(lua_code)
    output.write(os.linesep)
    num_emits += 1

  return num_emits

def parse_rule(override, value):
  "Parse and validate a single Emit table override"
  if override not in EMIT_FIELDS:
    choices = ", ".join(EMIT_FIELDS)
    raise ValueError(f"Invalid override key: {override!r} not among {choices!r}")

  if override == "force":
    if value == "NONE":
      value = "Vis.DEFAULT_FORCE"
    else:
      value = "Vis.FORCE_" + value
  elif override == "limit":
    if value == "NONE":
      value = "Vis.DEFAULT_LIMIT"
    else:
      value = "Vis.LIMIT_" + value
  elif override == "blender":
    value = "Vis.BLEND_" + value
  elif override in ("life", "ulife"):
    if value.endswith("f"):
      value = f"Vis.frames2msec({value[:-1]})"
    else:
      value = int(value)
  elif override in ("count", "depth", "rad", "urad", "tag"):
    value = int(value)
  elif override in ("ux", "uy", "s", "us", "ds", "uds", "ur", "ug", "ub"):
    value = float(value)
  elif override in ("theta", "utheta"):
    value = (int(value) % 360) / 360 * 2 * math.pi

  return value

@contextmanager
def open_file_for_writing(path, append=False):
  "Obtain the output file specified by the given path, '-' for stdout"
  if path == "-":
    if append:
      logger.warning("Using append=True with stdout has no effect")
    # Use stdout directly; do not close it afterward
    yield sys.stdout
  else:
    with open(path, "at" if append else "wt", encoding="UTF-8") as fobj:
      yield fobj

def get_and_remove(collection, entry):
  "Obtain and remove the entry from the collection"
  value = collection[entry]
  del collection[entry]
  return value

def main():
  ap = argparse.ArgumentParser(
      usage=textwrap.dedent("""\
      %(prog)s PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s MSEC=PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s FRAMEf=PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s PATH,PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s MSEC=PATH,PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s FRAMEf=PATH,PATH... [-F PATH] [-o PATH] [-v]"""),
      epilog=textwrap.dedent("""
      A note on the four usage methods:
        First form: specify pixel image, without motion data
        Second form: specify pixel image, but include emit time
        Third form: specify pixel image, emit time in frames (and not msec)
        Fourth form: specify pixel image and motion image
        Fifth form: specify pixel image, motion image, and emit time
        Sixth form: specify pixel image, motion image, and emit frame

      All of these can be mixed and matched as desired. If msec is not
      specified, then the emits will be done through Emit:emit_now().
      Otherwise, Emit:emit_at will be used with either msec or
      Vis.frames2msec(frame).

      Motion data is interpreted as HSV (and only for the non-transparent
      pixels in the pixel data image). The Hue, an angle between 0 and 360,
      determines the direction of motion. The Saturation determines the
      tangential velocity. However, the Value component is not currently
      used. Equivalently, the image could be thought of as HSL (Hue,
      Saturation, Lightness) where the Lightness component is ignored.

      The following rule names are permitted when using -r,--rule:
        count           Number of particles to emit (by default, 1)
        ux, uy          Uncertainty of position
        depth           Screen depth override (all particles share this)
        s, us           Particle linear displacement along theta
        ds, uds         Particle linear speed (ds overrides motion data)
        rad, urad       Particle rad in pixels (by default, 1 and 0)
        theta, utheta   Particle angle (theta overrides motion data)
        life, ulife     Lifetime of the particle in milliseconds
        ur, ug, ub      Uncertainty in color
        force           Force function (NONE, FRICTION, GRAVITY)
        limit           Boundary function (NONE, BOX, SPRINGBOX)
        blender         Blending function (NONE, LINEAR, PARABOLIC,
                        QUADRATIC, SINE, NEGGAMA, EASING)
        tag             Particle tag (caution: all emits will share this)

      "Uncertainty" values are used to give uniform randomness to the various
      properties. For example, x=5,ux=5 means the particles will be spawned
      at an x position between 0 and 10 (x-ux..x+ux).

      The -t,--threshold argument accepts one of two threshold rules:
        LESS NUMBER     Emit pixel if all colors are greater than NUMBER
        MORE NUMBER     Emit pixel if all colors are less than NUMBER

      The -O,--optimizer argument accepts one of two optimization rules:
        HSTRIP          Reduce consecutive emits on the same row
        HSTRIP2         Combination of HSTRIP and DESAMPLE2
        HSTRIP4         Combination of HSTRIP and DESAMPLE4
        VSTRIP          Reduce consecutive emits on the same column
        VSTRIP2         Combination of VSTRIP and DESAMPLE2
        VSTRIP4         Combination of VSTRIP and DESAMPLE4
        DESAMPLE2       Reduce number of emits to a half of the total
        DESAMPLE4       Reduce number of emits to a quarter of the total
      Note that this necessarily uses ux,uy. Passing `-r ux=value` or
      `-r uy=value` will override these optimizations.

      -T,--table-func can be used to wrap the Emit table with arbitrary
      code. For example, -T "merge_defaults({{{table}}})" will call
      merge_defaults() on the generated Emit table. Its return value
      will then be used for the final Emit table."""),
      formatter_class=argparse.RawDescriptionHelpFormatter)
  ag = ap.add_argument_group("input specification")
  ag.add_argument("image_path", nargs="*", metavar="[MSEC=]PATH[,PATH]",
      help="path to specific image(s), with optional motion and time data")
  ag.add_argument("-F", "--file", metavar="PATH",
      help="read file paths from %(metavar)s")
  ag = ap.add_argument_group("emit and behavior overrides")
  ag.add_argument("-r", "--rule", metavar="NAME=VALUE", action="append",
      help="override specific entries in the Emit table; see below for list")
  ag.add_argument("-s", "--size", metavar="WIDTH[,x ]HEIGHT",
      help="override destination extents; image is centered if smaller")
  ag.add_argument("-S", "--no-scale", action="store_true",
      help="do not scale images to fit output resolution; simply center")
  ag.add_argument("-b", "--blank", metavar=("R","G","B"), nargs=3,
      action="append", help="treat color R G B as transparent (multi)")
  ag.add_argument("-t", "--threshold", metavar=("RULE","VALUE"), nargs=2,
      action="append",
      help="omit pixels matching RULE with VALUE (multi; see below)")
  ag.add_argument("-O", "--optimizer", metavar="RULE", default="NONE",
      help="reduce the number of Emit calls using %(metavar)s (see below)")
  ag.add_argument("-n", "--native", action="store_true",
      help="output using native Vis.emit instead of using Emit class")
  ag.add_argument("-T", "--table-func", metavar="LUA", default="{{{table}}}",
      help="override emit table generation (default: %(default)r)")
  ag = ap.add_argument_group("output specification")
  ag.add_argument("-o", "--output", metavar="PATH", default="-",
      help="write Lua commands to %(metavar)s; default is stdout")
  ag.add_argument("-a", "--append", action="store_true",
      help="append Lua commands to the output file instead of overwriting")
  ag.add_argument("-B", "--pre-script", metavar="PATH",
      help="prepend the contents of %(metavar)s to the output ([B]efore)")
  ag.add_argument("-A", "--post-script", metavar="PATH",
      help="append the contents of %(metavar)s to the output ([A]fter)")
  ap.add_argument("-v", "--verbose", action="store_true",
      help="enable verbose diagnostics")

  args = ap.parse_args()
  if args.verbose:
    logger.setLevel(logging.DEBUG)

  image_list = gather_images(args)
  if not image_list:
    ap.error("Nothing to do")

  window_size = None
  if args.size:
    width_s, height_s = "0", "0"
    if args.size.count(",") == 1:
      width_s, height_s = args.size.split(",")
    elif args.size.count("x") == 1:
      width_s, height_s = args.size.split("x")
    elif args.size.count(" ") == 1:
      width_s, height_s = args.size.split()
    else:
      ap.error(f"-s,--size {args.size} has unknown format")
    window_size = (int(width_s), int(height_s))

  thresholds = []
  if args.threshold:
    for rule in args.threshold:
      thresholds.append((threshold_arg_to_func(rule[0]), int(rule[1])))

  optimizer = Optimizer.NONE
  if args.optimizer:
    optimizer = optimizer_arg_to_func(args.optimizer)

  # Determine if we really need to regenerate the file
  curr_settings = repr(sys.argv)
  if args.output != "-" and os.path.exists(args.output):
    if os.stat(args.output).st_size > 0:
      with open(args.output, "rt", encoding="UTF-8") as fobj:
        line = next(fobj).strip()
      if line.startswith(GEN_SETTINGS_COMMENT):
        gen_settings = line.split(GEN_SETTINGS_DELIM, 1)[1]
        if gen_settings == curr_settings:
          logger.info("Already generated %s; skipping", args.output)
          raise SystemExit(0)

  # Parse -r,--rule arguments into the overrides dict
  overrides = {}
  if args.rule:
    for rule in args.rule:
      if rule.count("=") != 1:
        ap.error(f"Invalid rule {rule!r}: delimiter missing or duplicated")
      override, value = rule.split("=")
      overrides[override] = parse_rule(override, value)

  with open_file_for_writing(args.output, append=args.append) as output:
    logger.info("Processing %d image record(s) to %s as %s", len(image_list),
        args.output, output.name)

    try:
      logger.debug("Writing \"%s\" to %s", curr_settings, args.output)
      output.write(f"{GEN_SETTINGS_COMMENT}{curr_settings}{os.linesep}")

      # Prepend the contents of the pre-script
      if args.pre_script:
        output.write(f"-- Content of {args.pre_script}:{os.linesep}")
        with open(args.pre_script, "rt", encoding="UTF-8") as fobj:
          content = fobj.read()
        output.write(content)
        if not content.endswith(os.linesep):
          output.write(os.linesep)
        output.write(f"-- Emit rules begin here:{os.linesep}")

      # Perform the actual work: interpret the image and generate Lua
      total_emits = 0
      for rid, pixel_image in enumerate(image_list):
        logger.debug("Processing record %d of %d", rid+1, len(image_list))
        motion_image = image_list[pixel_image]["motion"]
        ftime = image_list[pixel_image]["ftime"]
        # TODO: Merge identical Emits from subsequent frames (think MPEG)
        pixel_data, motion_data = extract_pixel_and_motion_data(
            pixel_image, motion_image, window_size, blank=args.blank,
            thresholds=thresholds, noscale=args.no_scale)
        logger.info("Emitting Lua for %s", pixel_image)
        this_frame_emits = emit_lua(pixel_data, motion_data, ftime, output,
            optimizer=optimizer, native=args.native, **overrides)
        total_emits += this_frame_emits
        if optimizer and optimizer != Optimizer.NONE:
          logger.debug("Optimized %d pixels to %d emits (%02.02f%%) reduction",
              len(pixel_data), this_frame_emits,
              100 - this_frame_emits / len(pixel_data) * 100)

      # Append the contents of the post-script
      if args.post_script:
        output.write(os.linesep)
        output.write(f"-- Content of {args.post_script}:{os.linesep}")
        with open(args.post_script, "rt", encoding="UTF-8") as fobj:
          content = fobj.read()
        output.write(content)
        if not content.endswith(os.linesep):
          output.write(os.linesep)

    except ValueError as err:
      logger.error(err)
      raise SystemExit(1) from err

  logger.info("Generated %d emits across all inputs", total_emits)

if __name__ == "__main__":
  main()

# vim: set ts=2 sts=2 sw=2:
