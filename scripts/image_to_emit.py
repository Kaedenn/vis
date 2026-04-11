#!/usr/bin/env python3

"""
Convert image(s) to Lua emit calls
"""

# FIXME: This generates far too many Emit calls. Add some intelligent processing
# to aggregate contiguous strips of identical color (run-length-encoding?) and
# emit those pixels together using ux/uy offsets.

# TODO: Implement a form of temporal compression to reduce Emit calls: the
# lifetime of a particle should be increased if the pixel is unchanged on the
# subsequent frame. Instead of generating two emits, generate one with the frame
# duration added to the lifetime.

import argparse
from contextlib import contextmanager
import enum
import logging
import math
import os
import sys
import textwrap

from PIL import Image

logging.basicConfig(format="%(module)s:%(lineno)s: %(levelname)s: %(message)s",
                    level=logging.INFO)
logger = logging.getLogger(__name__)

EMIT_FIELDS = ("count", "ux", "uy", "depth", "s", "us", "ds", "uds",
      "rad", "urad", "theta", "utheta", "life", "ulife", "ur", "ug", "ub",
      "force", "limit", "blender", "tag")

class Optimizer(enum.Enum):
  "Method to use for reducing the number of emit calls"
  NONE = enum.auto()
  HSTRIP = enum.auto()
  VSTRIP = enum.auto()
  DESAMPLE2 = enum.auto()
  DESAMPLE4 = enum.auto()

class ThresholdFunc(enum.Enum):
  "Threshold function used to prune unneeded pixels from the output"
  NONE = enum.auto()
  LESS = enum.auto()
  MORE = enum.auto()

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

def evaluate_optimizer(pixel_data, motion_data, func):
  "Optimize the pixel and motion data to reduce the number of Emit calls"
  if func == Optimizer.NONE:
    return pixel_data, motion_data
  if func not in (Optimizer.DESAMPLE2, Optimizer.DESAMPLE4):
    logger.warning("Optimizer function %s not yet implemented", func)
  return pixel_data, motion_data

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
    if rgba[3] == 0:
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

  logger.debug("Emitting %d of %d pixels (%02.02f%%)", len(pixel_data),
      width*height, len(pixel_data)/(width*height)*100)
  pixel_values = list(pixel_data.values())
  pixel_colors = set(pixel_values)
  total_pixels = sum(pixel_values.count(color) for color in pixel_colors)
  logger.debug("%d pixel colors appear an average of %d times each",
      len(pixel_colors), total_pixels // len(pixel_colors))

  return pixel_data, motion_data

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

def emit_lua(pixel_data, motion_data, ftime, output, optimizer=Optimizer.NONE,
    native=False, **overrides):
  "Build the final Lua code"
  num_emits = 0
  emit_fields = dict.fromkeys(EMIT_FIELDS, 0)
  emit_fields["count"] = 1
  emit_fields["rad"] = 1
  emit_fields["life"] = 200
  if optimizer == Optimizer.HSTRIP:
    logger.error("HSTRIP not implemented yet; ignoring")
  elif optimizer == Optimizer.VSTRIP:
    logger.error("VSTRIP not implemented yet; ignoring")
  for pixel_x, pixel_y in pixel_data:
    if optimizer == Optimizer.DESAMPLE2:
      if pixel_x % 2 or pixel_y % 2:
        continue
    elif optimizer == Optimizer.DESAMPLE4:
      if pixel_x % 4 or pixel_y % 4:
        continue
    pixel = pixel_data[(pixel_x, pixel_y)]
    motion_hsv = motion_data.get((pixel_x, pixel_y), (0, 0, 0))
    emit_fields["x"] = pixel_x
    emit_fields["y"] = pixel_y
    emit_fields["theta"] = motion_hsv[0] / 360 * 2 * math.pi
    emit_fields["ds"] = motion_hsv[1] / 255
    emit_fields["r"] = pixel[0] / 255
    emit_fields["g"] = pixel[1] / 255
    emit_fields["b"] = pixel[2] / 255

    emit_fields.update(**overrides)
    if native:
      count = get_and_remove(emit_fields, "count")
      table = ", ".join(f"{key}={value}" for key, value in emit_fields.items())
      if ftime is not None:
        lua_code = f"Vis.emit(Vis.flist, {count}, {ftime}, {{{table}}})"
      else:
        lua_code = f"Vis.emitnow(Vis.script, {count}, {{{table}}})"
    else:
      table = ", ".join(f"{key}={value}" for key, value in emit_fields.items())
      if ftime is not None:
        lua_code = f"Emit:new({{{table}}}):emit_at({ftime})"
      else:
        lua_code = f"Emit:new({{{table}}}):emit_now()"

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
  elif override in ("count", "depth", "rad", "urad", "life", "ulife", "tag"):
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
        VSTRIP          Reduce consecutive emits on the same column
        DESAMPLE2       Reduce number of emits to a half of the total
        DESAMPLE4       Reduce number of emits to a quarter of the total
      Note that this necessarily uses ux,uy. Passing `-r ux=value` or
      `-r uy=value` will override these optimizations."""),
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

  field_size = None
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
    field_size = (int(width_s), int(height_s))

  thresholds = []
  if args.threshold:
    for rule in args.threshold:
      thresholds.append((threshold_arg_to_func(rule[0]), int(rule[1])))

  optimizer = Optimizer.NONE
  if args.optimizer:
    optimizer = optimizer_arg_to_func(args.optimizer)

  with open_file_for_writing(args.output, append=args.append) as output:
    logger.info("Processing %d image record(s) to %s", len(image_list),
        output.name)

    overrides = {}
    try:
      # Parse -r,--rule arguments into the overrides dict
      if args.rule:
        for rule in args.rule:
          if rule.count("=") != 1:
            ap.error(f"Invalid rule {rule!r}: delimiter missing or duplicated")
          override, value = rule.split("=")
          overrides[override] = parse_rule(override, value)

      # Prepend the contents of the pre-script
      if args.pre_script:
        output.write(f"# Content of {args.pre_script}:{os.linesep}")
        with open(args.pre_script, "rt", encoding="UTF-8") as fobj:
          content = fobj.read()
        output.write(content)
        if not content.endswith(os.linesep):
          output.write(os.linesep)
        output.write(f"# Emit rules begin here:{os.linesep}")

      # Perform the actual work: interpret the image and generate Lua
      total_emits = 0
      for rid, pixel_image in enumerate(image_list):
        logger.debug("Processing record %d of %d", rid+1, len(image_list))
        motion_image = image_list[pixel_image]["motion"]
        ftime = image_list[pixel_image]["ftime"]
        pixel_data, motion_data = extract_pixel_and_motion_data(
            pixel_image, motion_image, field_size, blank=args.blank,
            thresholds=thresholds, noscale=args.no_scale)
        logger.info("Emitting Lua for %s", pixel_image)
        total_emits += emit_lua(pixel_data, motion_data, ftime, output,
            optimizer=optimizer, native=args.native, **overrides)

      # Append the contents of the post-script
      if args.post_script:
        output.write(os.linesep)
        output.write(f"# Content of {args.post_script}:{os.linesep}")
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
