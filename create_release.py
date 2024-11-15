#!/usr/bin/env python3
#
# Written by Johan Henriksson. Copyright (C) 2014-2021.
#
import sys
import os
import subprocess


if len(sys.argv) < 2:
    print("Usage: ./create_release.py VERSION")
    print("Available versions:")
    p = subprocess.Popen(['git', 'tag'], stdout=subprocess.PIPE, text=True)
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

#  Remove some files from the archive
os.system("tar --delete -f %s 'gede-%s/create_release.py'" % (filename, version))
os.system("tar --delete -f %s 'gede-%s/tagit.py'" % (filename, version))
os.system("tar --delete -f %s --wildcards 'gede-%s/relop_*'" % (filename, version))
os.system("tar --delete -f %s 'gede-%s/.gitignore'" % (filename, version))
os.system("tar --delete -f %s 'gede-%s/startpage'" % (filename, version))
os.system("tar --delete -f %s 'gede-%s/doc'" % (filename, version))



os.system("xz -9 %s" % (filename))
print("%s.xz created" % (filename)) 



