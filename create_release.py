#!/usr/bin/env python

import sys
import os
import subprocess


if len(sys.argv) < 2:
    print("Usage: ./create_release.py VERSION")
    print("Available versions:")
    p = subprocess.Popen(['git', 'tag'], stdout=subprocess.PIPE)
    out, err = p.communicate()
    errcode = p.returncode
    for line in out.split('\n'):
        if line.find("rel-") == 0:
            print(" %s" % (line[4:]))
    exit(1)
    
# Parse arguments
version = sys.argv[1]

filename = "gede-%s.tar" % (version)

if os.path.exists(filename):
    os.remove(filename)
if os.path.exists(filename + ".xz"):
    os.remove(filename + ".xz")

os.system("git archive --format tar --prefix=gede-%s/ --output %s rel-%s" % (version, filename, version))
os.system("xz -9 %s" % (filename))
print("%s.xz created" % (filename)) 
