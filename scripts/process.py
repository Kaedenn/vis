#!/usr/bin/env python

import argparse
import glob
import os
import re
import subprocess
import sys

"""
process.py: convert a sequence of images into a proper AVI with ffmpeg
"""

FFMPEG = os.environ.get('FFMPEG', 'ffmpeg')
CONVERT = os.environ.get('CONVERT', 'convert')

SUFFIX_FMT = "_%04d.png"
SUFFIX_GLOB = "_[0-9][0-9][0-9][0-9].png"
SUFFIX_PATTERN = re.compile('/_[0-9]{4}\.png$/')

def verbose(args, string, *fmt_args):
    if args.verbose:
        sys.stdout.write(string % fmt_args)
        sys.stdout.write('\n')

def progress(args, string, *fmt_args):
    if args.verbose or args.progress:
        sys.stdout.write(string % fmt_args)
        sys.stdout.write('\r')

def do_flip(args):
    in_glob = args.in_prefix + SUFFIX_GLOB
    verbose(args, "flipping %s to %s", in_glob, args.out_prefix + SUFFIX_FMT)
    in_files = glob.glob(in_glob)
    in_files.sort()
    verbose(args, "found %s input files", len(in_files))
    if args.skip > 0:
        del in_files[:args.skip-1]
    if len(in_files) == 0:
        raise RuntimeError("found no files to process; quitting")
    error = False
    for octr, filename in enumerate(in_files):
        outf = os.path.join(args.out_prefix + (SUFFIX_FMT % (octr,)))
        if subprocess.call([CONVERT, '-flip', filename, outf]) != 0:
            sys.stderr.write("Failed to flip %s to %s, skipping...\n" %
                             (filename, outf))
            error = True
        progress(args, "%02d%%: %s to %s", octr*100/len(in_files),
                 filename, outf)
    progress(args, "\n")
    if error:
        raise SystemExit(1)

def do_encode(args):
    if args.skip > 0:
        sys.stderr.write("Warning: \"encode\" ignores '--skip', use " +
                         "\"flip --skip\" instead\n")
    in_fmt = args.in_prefix + SUFFIX_FMT
    ou_avi = args.out_prefix
    ffmpeg = [FFMPEG, '-framerate', str(args.fps), '-i', in_fmt]
    if args.add_encode is not None:
        for piece in args.add_encode:
            ffmpeg.extend(['-i', piece])
    ffmpeg.extend(['-shortest', '-c:v', 'libx264', ou_avi])
    if args.ffargs is not None:
        ffmpeg.extend(args.ffargs.split())
    if os.path.exists(ou_avi):
        sys.stderr.write("Warning: removing existing file %s!\n" % (ou_avi,))
        os.remove(ou_avi)
    if subprocess.call(ffmpeg) != 0:
        sys.stderr.write("Failed to encode %s to %s\n", in_fmt, ou_avi)
        raise SystemExit(1)

S_FFMPEG_NOT_INSTALLED = """\
Error! ffmpeg not installed! Please be sure you install the ffmpeg version and
*NOT* the Libav version!"""

S_FFMPEG_WRONG_VERSION = """\
Error! ffmpeg is the wrong distribution! This program only works with the
official release of ffmpeg, *NOT* the Libav release!"""

S_FFMPEG_INSTALLING = """\
There are two versions of ffmpeg depending on what distribution you have
installed (Ubuntu 14, Ubuntu 15, Debian, etc). This project requires the
ffmpeg version and *NOT* the Libav version. Please refer to
    http://stackoverflow.com/questions/9477115/w/9477756#9477756
for an explanation and download the official ffmpeg (not the fake one!) at
    https://ffmpeg.org/download.html#LinuxBuilds
"""

S_CONVERT_NOT_INSTALLED = """\
Error! ImageMagick not installed! Please install it from the web site or from
your local package manager or set CONVERT=/usr/local/bin/convert (or wherever
it is) before using this program.
"""

S_CONVERT_ERROR = """\
Error! There is something wrong with your install of ImageMagick! The command
    %s -version
returned with non-zero exit status! Please fix ImageMagick or set
CONVERT=/usr/local/bin/convert (or wherever it is) before using this program.
"""

class FFmpegError(Exception):
    def __str__(self):
        return S_FFMPEG_INSTALLING

class FFmpegNotInstalledError(FFmpegError):
    def __str__(self):
        return "%s\n\n%s" % (S_FFMPEG_NOT_INSTALLED,
                             FFmpegError.__str__(self))

class FFmpegWrongVersionError(FFmpegError):
    def __str__(self):
        return "%s\n\n%s" % (S_FFMPEG_WRONG_VERSION,
                             FFmpegError.__str__(self))

class ConvertError(Exception):
    def __str__(self):
        return ''

class ConvertNotInstalledError(ConvertError):
    def __str__(self):
        return S_CONVERT_NOT_INSTALLED

class ConvertGenericError(ConvertError):
    def __str__(self):
        return S_CONVERT_ERROR % (CONVERT,)

def check_ffmpeg():
    FFMPEG_PRESENT = 0
    FFMPEG_NOT_INSTALLED = 1
    FFMPEG_WRONG_VERSION = 2

    status = FFMPEG_PRESENT
    try:
        p = subprocess.Popen([FFMPEG, '-version'], stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        out, err = p.communicate()
        if ' libav ' in out.splitlines()[0].lower():
            status = FFMPEG_WRONG_VERSION
    except OSError, _:
        status = FFMPEG_NOT_INSTALLED

    if status == FFMPEG_NOT_INSTALLED:
        raise FFmpegNotInstalledError()
    elif status == FFMPEG_WRONG_VERSION:
        raise FFmpegWrongVersionError()

def check_imagemagick():
    IMAGEMAGICK_PRESENT = 0
    IMAGEMAGICK_NOT_INSTALLED = 1
    IMAGEMAGICK_ERROR = 2

    status = IMAGEMAGICK_PRESENT
    try:
        s = subprocess.call([CONVERT, '-version'], stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
        if s != 0:
            status = IMAGEMAGICK_ERROR
    except OSError, _:
        status = IMAGEMAGICK_NOT_INSTALLED

    if status == IMAGEMAGICK_NOT_INSTALLED:
        raise ConvertNotInstalledError()
    elif status == IMAGEMAGICK_ERROR:
        raise ConvertGenericError()

def parse_args(argv):
    USAGE = "%(prog)s <\"flip\", \"encode\"> [args] <in> <out>"
    EPILOG = """examples:
    %(prog)s flip output/bowser1 output/bowser2
        Flips output/bowser1_%%04d.png vertically and writes the results to
        output/bowser2_%%04d.png.

    %(prog)s flip -s 1 output/bowser1 output/bowser2
        Exactly as above, skipping the first frame: output/bowser_0000.png

    %(prog)s encode output/bowser2 output/bowser.avi --add-encode bowser.wav
        Encodes output/bowser2_%%04d.png to output/bowser.avi, also including
        the file bowser.wav.

    %(prog)s encode -s 2 output/bowser2 output/bowser.avi
        Exactly as above, skipping the first two frames, with no audio"""
    p = argparse.ArgumentParser(usage=USAGE, epilog=EPILOG,
                                formatter_class=
                                    argparse.RawDescriptionHelpFormatter)
    p.add_argument("action", type=str, choices=['flip', 'encode'],
                   help="operation to perform: one of 'flip', 'encode'")
    p.add_argument("in_prefix", type=str,
                   help="see the 'examples' section below")
    p.add_argument("out_prefix", type=str,
                   help="see the 'examples' section below")
    p.add_argument("--add-encode", action="append",
                   help="add another input to ffmpeg")
    p.add_argument("--ffargs", metavar='STR',
                   help="additional arguments to pass to ffmpeg")
    p.add_argument("-s", "--skip", type=int, default=0, metavar='FRAMES',
                   help="skip the first <FRAMES> images")
    p.add_argument("-f", "--fps", type=int, default=30, metavar='FPS',
                   help="force frames-per-second to a value other than 30")
    p.add_argument("-v", "--verbose", action="store_true")
    p.add_argument("-p", "--progress", action="store_true")
    return p.parse_args(argv)

def main(argv):
    args = parse_args(argv)
    if args.action == 'flip':
        check_imagemagick() # termination point
        do_flip(args)
    if args.action == 'encode':
        check_ffmpeg() # termination point
        do_encode(args)

def do_test(argv):
    raise NotImplementedError("--test is not yet implemented")

if __name__ == "__main__":
    for i,a in enumerate(sys.argv):
        if a == '--test':
            del sys.argv[i]
            do_test(sys.argv)
            raise SystemExit(0)
    try:
        main(sys.argv[1:])
    except FFmpegError as e:
        sys.stderr.write("FFmpeg error: %s\n" % (e,))
        raise SystemExit(1)
    except ConvertError as e:
        sys.stderr.write("ImageMagick convert error: %s\n" % (e,))
        raise SystemExit(1)
    except RuntimeError as e:
        sys.stderr.write("Runtime error: %s\n" % (e,))
        raise SystemExit(1)
    except SystemExit as e:
        if e.code != 0:
            sys.stderr.write("Terminating!\n")
        raise

