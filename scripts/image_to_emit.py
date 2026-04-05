#!/usr/bin/env python3

"""
Convert image(s) to Lua emit calls
"""

import argparse
from contextlib import contextmanager
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

def parse_image_entry(entry):
  "Parse a single image entry"
  msec, ppath, mpath = None, entry, None
  if "=" in entry:
    msec_s, ppath = entry.split("=")
    msec = int(msec_s)
  if "," in ppath:
    ppath, mpath = ppath.split(",")
    if not os.path.isfile(mpath):
      logger.warning("Entry %s motion path %s is not a file", entry, mpath)
      mpath = None
  if not os.path.isfile(ppath):
    logger.warning("Entry %s pixel path %s is not a file", entry, ppath)
    return None, None, None
  return ppath, mpath, msec

def gather_images(args):
  "Obtain a list of all images to process"
  results = {}
  if args.image_path:
    for image_path in args.image_path:
      ppath, mpath, msec = parse_image_entry(image_path)
      if ppath is not None:
        results[ppath] = {"motion": mpath, "msec": msec}
  if args.file:
    with open(args.file, "rt", encoding="UTF-8") as fobj:
      for image_path in fobj.read().splitlines():
        ppath, mpath, msec = parse_image_entry(image_path)
        if ppath is not None:
          results[ppath] = {"motion": mpath, "msec": msec}
  logger.debug("Read %d image record(s) from inputs: %s", len(results), results)

  return results

def extract_pixel_and_motion_data(image, motion, window_size=(None, None)):
  "Generate Lua commands for the given image (or pair) and write to the output"
  width, height = 0, 0
  pixel_data = {}
  motion_data = {}

  img = Image.open(image)
  if img.mode != "RGBA":
    img.convert("RGBA")
  if None not in window_size:
    img = resize_image_centered(img, *window_size)
  width, height = img.size
  for pidx, rgba in enumerate(img.get_flattened_data()):
    pixel_x = pidx % width
    pixel_y = pidx // width % height
    if rgba[3] > 0:
      pixel_data[(pixel_x, pixel_y)] = rgba[:3]
  img.close()

  if motion:
    img = Image.open(motion)
    if None not in window_size:
      img = resize_image_centered(img, *window_size)
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

  logger.debug("Emitting %d of %d pixels (%02.02f%%)", len(pixel_data), width*height,
      len(pixel_data)/(width*height)*100)
  pixel_values = list(pixel_data.values())
  pixel_colors = set(pixel_values)
  for color in pixel_colors:
    logger.debug("Color %s appears %d times", color, pixel_values.count(color))

  return pixel_data, motion_data

def resize_image_centered(image: Image.Image, width: int|None, height: int|None) \
    -> Image.Image:
  # Ensure we have an alpha channel so transparency works correctly
  if image.mode != "RGBA":
     image = image.convert("RGBA")

  if width is None or height is None:
    return image

  # Compute scale factor to fit within target while preserving aspect ratio
  scale = min(width / image.width, height / image.height)

  new_w = int(round(image.width * scale))
  new_h = int(round(image.height * scale))

  resized = image.resize((new_w, new_h), Image.NEAREST)

  # Create transparent background
  result = Image.new("RGBA", (width, height), (0, 0, 0, 0))

  # Compute centered position (can be negative if oversized, but we scaled to fit)
  x = (width - new_w) // 2
  y = (height - new_h) // 2

  # Paste with alpha mask to preserve transparency
  result.paste(resized, (x, y), resized)

  return result

def emit_lua(pixel_data, motion_data, msec, output, **overrides):
  "Build the final Lua code"
  emit_fields = dict.fromkeys(EMIT_FIELDS, 0)
  emit_fields["count"] = 1
  emit_fields["rad"] = 1
  emit_fields["life"] = 1000
  emit_func, emit_args = "emit_at", "0"
  for pixel_x, pixel_y in pixel_data:
    pixel = pixel_data[(pixel_x, pixel_y)]
    motion_hsv = motion_data.get((pixel_x, pixel_y), (0, 0, 0))
    emit_fields["x"] = pixel_x
    emit_fields["y"] = pixel_y
    assert isinstance(pixel_x, int), f"pixel_x is {type(pixel_x)} not int"
    assert isinstance(pixel_y, int), f"pixel_y is {type(pixel_y)} not int"
    emit_fields["theta"] = motion_hsv[0] / 360 * 2 * math.pi
    emit_fields["ds"] = motion_hsv[1] / 255
    emit_fields["r"] = pixel[0] / 255
    emit_fields["g"] = pixel[1] / 255
    emit_fields["b"] = pixel[2] / 255
    if msec is not None:
      emit_args = f"{msec}"
    else:
      emit_func = "emit_now"

    emit_fields.update(**overrides)
    emit_table = ", ".join(f"{key}={value}" for key, value in emit_fields.items())

    lua_code = f"""emit_obj = Emit:new({{{emit_table}}})
emit_obj:{emit_func}({emit_args})"""
    output.write(lua_code)
    output.write(os.linesep)

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

def main():
  ap = argparse.ArgumentParser(
      usage=textwrap.dedent("""\
      %(prog)s PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s MSEC=PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s PATH,PATH... [-F PATH] [-o PATH] [-v]
      usage: %(prog)s MSEC=PATH,PATH... [-F PATH] [-o PATH] [-v]"""),
      epilog=textwrap.dedent("""
      A note on the four usage methods:
        First form: specify single pixel data image, without motion data
        Second form: specify single pixel data image, but include emit time
        Third form: specify pixel image and motion image
        Fourth form: specify pixel image, motion image, and emit time

      All of these can be mixed and matched as desired. If msec is not
      specified, then the emits will be done through Emit:emit_now().
      Otherwise, Emit:emit_at(msec) will be used.

      Motion data is interpreted as HSV (and only for the non-transparent
      pixels in the pixel data image). The Hue, an angle between 0 and 360,
      determines the direction of motion. The Saturation determines the
      tangential velocity. However, the Value component is not currently
      used. Equivalently, the image could be thought of as HSL (Hue,
      Saturation, Lightness) where the Lightness component is ignored.

      The following rule names are permitted when using -r,--rule:
        count           Number of particles to emit (by default, 1)
        ux, uy          Uncertainty of position
        depth           Screen depth override
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
        """),
      formatter_class=argparse.RawDescriptionHelpFormatter)
  ag = ap.add_argument_group("input specification")
  ag.add_argument("image_path", nargs="*", metavar="[MSEC=]PATH[,PATH]",
      help="path to specific image(s), with optional motion and time data")
  ag.add_argument("-F", "--file", metavar="PATH",
      help="read file paths from %(metavar)s")
  ag = ap.add_argument_group("emit overrides")
  ag.add_argument("-r", "--rule", metavar="NAME=VALUE", action="append",
      help="override specific entries in the Emit table; see below for list")
  ag.add_argument("-s", "--size", metavar="WIDTH,HEIGHT",
      help="override destination extents; image is centered if smaller")
  ap.add_argument("-a", "--append", action="store_true",
      help="append Lua commands to the output file instead of overwriting")
  ap.add_argument("-o", "--output", metavar="PATH", default="-",
      help="write Lua commands to %(metavar)s; default is stdout")
  ap.add_argument("-v", "--verbose", action="store_true",
      help="enable verbose diagnostics")

  args = ap.parse_args()
  if args.verbose:
    logger.setLevel(logging.DEBUG)

  image_list = gather_images(args)
  if not image_list:
    ap.error("Nothing to do")

  field_width, field_height = None, None
  if args.size:
    if args.size.count(",") != 1:
      ap.error("-s,--size missing comma; expecting NUMBER,NUMBER")
    width_s, height_s = args.size.split(",")
    field_width = int(width_s)
    field_height = int(height_s)

  with open_file_for_writing(args.output, append=args.append) as output:  
    logger.info("Processing %d image record(s) to %s", len(image_list),
        output.name)

    overrides = {}
    try:
      if args.rule:
        for rule in args.rule:
          if rule.count("=") != 1:
            ap.error(f"Invalid rule {rule!r}: delimiter missing or duplicated")
          override, value = rule.split("=")
          overrides[override] = parse_rule(override, value)

      total_emits = 0
      for rid, pixel_image in enumerate(image_list):
        logger.debug("Processing record %d of %d", rid+1, len(image_list))
        motion_image = image_list[pixel_image]["motion"]
        msec = image_list[pixel_image]["msec"]
        pixel_data, motion_data = extract_pixel_and_motion_data(pixel_image, motion_image, (field_width, field_height))
        total_emits += len(pixel_data)
        emit_lua(pixel_data, motion_data, msec, output, **overrides)

    except ValueError as err:
      logger.error(err)
      raise SystemExit(1)
  logger.info("Generated %d emits across all inputs", total_emits)

if __name__ == "__main__":
  main()

# vim: set ts=2 sts=2 sw=2:
