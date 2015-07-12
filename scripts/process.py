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

SUFFIX_FMT = "_%04d.png"
SUFFIX_GLOB = "_[0-9][0-9][0-9][0-9].png"
SUFFIX_PATTERN = re.compile('/_[0-9]{4}\.png$/')
VERBOSE = False

def verbose(string, *fmt_args):
    if VERBOSE:
        sys.stdout.write(string % fmt_args)
        sys.stdout.write('\n')

def verbose_decor(fn):
    def wrapper(*args, **kwargs):
        s = "%s(%s, %s) = %s"
        arg_s = args[0] if len(args) == 1 else args
        try:
            value = fn(*args, **kwargs)
            verbose(s, fn, arg_s, kwargs, value)
            return value
        except Exception, e:
            verbose(s, fn, arg_s, kwargs, e)
            raise
    return wrapper

def progress(args, string, *fmt_args):
    if args.verbose or args.progress:
        sys.stdout.write(string % fmt_args)
        sys.stdout.write('\r')

@verbose_decor
def split_ffargs(arg):
    filepath = arg
    fileargs = None
    if '::' in arg:
        filepath, fileargs = arg.split('::', 1)
        fileargs = fileargs.split()
    return fileargs, filepath

@verbose_decor
def prep_argspec(fileargs, filepath, mode):
    if fileargs is None and mode == "-i":
        fileargs = ['-i']
    elif fileargs is not None and mode == '-i':
        fileargs.append('-i')
    elif fileargs is None and mode is None:
        fileargs = []
    elif fileargs is not None and mode is None:
        pass # fileargs = fileargs
    return fileargs + [filepath]

@verbose_decor
def split_ffargs_in(pathspec, fmt=''):
    fileargs, filepath = split_ffargs(pathspec)
    return prep_argspec(fileargs, filepath + fmt, '-i')

@verbose_decor
def split_ffargs_out(pathspec, fmt=''):
    fileargs, filepath = split_ffargs(pathspec)
    return prep_argspec(fileargs, filepath + fmt, None)

@verbose_decor
def do_encode(args):
    in_argspec = split_ffargs_in(args.finput, fmt=SUFFIX_FMT)
    out_argspec = split_ffargs_out(args.output)
    if '-framerate' not in in_argspec:
        in_argspec = ['-framerate', '30'] + in_argspec
    ffmpeg = [FFMPEG] + in_argspec
    if args.add_input is not None:
        for piece in args.add_input:
            ffmpeg.extend(split_ffargs_in(piece))
        ffmpeg.append('-shortest')
    ffmpeg.extend(['-c:v', 'libx264'] + out_argspec)
    if args.ffargs is not None:
        ffmpeg.extend(args.ffargs.split())
    verbose("Invoking %s", ' '.join(ffmpeg))
    if subprocess.call(ffmpeg) != 0:
        sys.stderr.write("Failed to encode %s to %s\n" % (in_argspec, out_argspec))
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

def parse_args(argv):
    USAGE = "%(prog)s [args] <finput>[::args] <output>[::args]"
    EPILOG = """\
Passing arguments to FFmpeg:
    Append "::<arguments>" to pass the arguments to FFmpeg. This applies to the
    input, output, and values to -i.

Examples:
    %(prog)s output/bowser output/bowser.avi
        Simplest case: encodes bowser_%%04d.png to output/bowser.avi.

    %(prog)s output/bowser2 output/bowser.avi -i bowser.wav
        Encodes output/bowser2_%%04d.png to output/bowser.avi, also including
        the file bowser.wav.

    %(prog)s "output/bowser::-dframes 100" bowser.avi
        As above, but skips the first 100 PNGs (-dframes skips over the number
        of frames given).

    %(prog)s output/bowser "bowser.mov::-target dvd" -i bowser.wav
        Reads output/bowser_%%04d.png and bowser.wav, writing to bowser.mov
        and encoding it as a DVD-playable media.

    %(prog)s "output/bowser::-dframes 100" "bowser.avi::-framerate 32" -i \
"bowser.wav::-s 00:00:02.50"
        Reads output/bowser_%%04d.png skipping the first 100 PNGs, reads
        bowser.wav skipping the first 2.5 seconds of audio, and writes the
        combination to bowser.avi at 32 frames per second.
"""
    p = argparse.ArgumentParser(usage=USAGE, epilog=EPILOG,
                                formatter_class=
                                    argparse.RawDescriptionHelpFormatter)
    p.add_argument("finput", type=str, help="see the 'examples' section below")
    p.add_argument("output", type=str, help="see the 'examples' section below")

    p.add_argument("-i", "--add-input", action="append", metavar="F[::args]",
                   help="add another input to ffmpeg")
    p.add_argument("--ffargs", metavar='STR',
                   help="arguments to append to the end of the FFmpeg command")

    p.add_argument("-v", "--verbose", action="store_true")
    return p.parse_args(argv)

def main(argv):
    global VERBOSE
    args = parse_args(argv)
    VERBOSE = args.verbose
    print VERBOSE, args, args.verbose
    check_ffmpeg() # termination point
    do_encode(args)

def do_test(argv):
    raise NotImplementedError("--test is not yet implemented")

if __name__ == "__main__":
    for i,a in enumerate(sys.argv):
        if a == '--test':
            del sys.argv[i]
            do_test(sys.argv)
    else:
        try:
            main(sys.argv[1:])
        except FFmpegError as e:
            sys.stderr.write("FFmpeg error: %s\n" % (e,))
            raise SystemExit(1)
        except RuntimeError as e:
            sys.stderr.write("Runtime error: %s\n" % (e,))
            raise SystemExit(1)

