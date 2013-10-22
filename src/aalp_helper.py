#!/usr/bin/env python

import fmbt
import sys
import urllib
import types
import traceback

def put(s):
    sys.stdout.write(urllib.quote(s) + "\n")

def names(iterable):
    try:
        results = ['"%s"' % (i,) for i in iterable]
    except TypeError:
        raise TypeError('Parameter of names() must be iterable, not %s as in "%s"' %
                        (type(iterable), iterable))
    put(", ".join(results))

line = sys.stdin.readline()
while line:
    block = urllib.unquote(line)
    fmbt.adapterlog("executing '''%s'''" % (block,))
    try:
        exec block
    except:
        put(traceback.format_exc())
    sys.stdout.flush()
    line = sys.stdin.readline()
