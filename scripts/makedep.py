#!/usr/bin/env python

import os
import os.path
import re
import sys

usrincl_re = re.compile('^#include "([^"]*)"')
sysincl_re = re.compile('^#include <([^>]*)>')
SRCDIR = os.path.join(os.path.dirname(sys.argv[0]), '..')
DEPDIR = os.path.join(SRCDIR, '.d')

def debug(s):
    if 'DEBUG' in os.environ or 'VIS_DEBUG' in os.environ:
        print(s)

def debug_dec(f):
    if 'DEBUG' in os.environ or 'VIS_DEBUG' in os.environ:
        def wrap(*args, **kwargs):
            result = f(*args, **kwargs)
            print("%s(%s, %s) -> %s" % (f, args, kwargs, result))
            return result
        return wrap
    else:
        return f

@debug_dec
def src2dep(path):
    return os.path.join(DEPDIR, os.path.basename(path))[:-1] + 'd'

@debug_dec
def dep2src(path):
    return os.path.join(SRCDIR, os.path.basename(path))[:-1] + 'c'

def sourcefiles():
    return (os.path.join(SRCDIR, f) for f in os.listdir(SRCDIR) if f[-2:] == '.c')

def headerfiles():
    return (os.path.join(SRCDIR, f) for f in os.listdir(SRCDIR) if f[-2:] == '.h')

def newerthan(f1, f2):
    if os.path.exists(f2):
        return os.stat(f1).st_ctime > os.stat(f2).st_ctime
    return True

def addresult(results, srcf, hdrf):
    if srcf not in results:
        results[srcf] = []
    results[srcf].append(hdrf)

@debug_dec
def gendepends(files):
    results = {}
    for fn,fh in ((f, open(f)) for f in files):
        for line in fh:
            m = usrincl_re.match(line.strip())
            if m is not None:
                addresult(results, fn, m.group(1))
    return results

def main(depfiles):
    srcs = sourcefiles()
    if depfiles is not None:
        srcs = [dep2src(p) for p in depfiles]
    debug("main(%s), using %s" % (depfiles, srcs))
    deps = (src2dep(f) for f in srcs)
    stale_srcs = []
    for f in srcs:
        if newerthan(f, src2dep(f)):
            debug("Source file updated %s: %s" % (f, src2dep(f)))
            stale_srcs.append(f)
        else:
            debug("Source file current %s: %s" % (f, src2dep(f)))
    debug("Regenerating %s" % (stale_srcs,))
    results = gendepends(stale_srcs)
    debug("Dependency results: %s" % (results,))
    for srcfile in results:
        depstr = "%s/%s: %s" % ('$(DIR)',
                                os.path.basename(srcfile),
                                ' '.join(results[srcfile]))
        open(src2dep(srcfile), 'w').write(depstr + "\n")
        debug(depstr)

if __name__ == "__main__":
    files = None
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    main(files)

