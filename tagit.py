#!/usr/bin/env python3
#
# Written by Johan Henriksson. Copyright (C) 2014-2021.
#
import sys
import os
import subprocess


with open("src/version.h", "r") as f:
    minor = ""
    major = ""
    patch = ""
    lines = f.readlines()
    for row in lines:
        row = row.lstrip()

        # Seperate the '#' character
        if len(row) > 0 and row[0] == '#':
            row = row[0] + ' ' + row[1:]

        # Split the row into words
        words = row.split()

        # Decode line
        if row.find("MAJOR") != -1:
            major = words[3]
        if row.find("MINOR") != -1:
            minor = words[3]
        if row.find("PATCH") != -1:
            patch = words[3]

    if minor != "" and major != "" and patch != "":
        tagName = ("v%s.%s.%s" % (major, minor, patch))
        print("Tagging with '%s'" % (tagName))
        os.system("git tag -a -m \"%s\" %s" % (tagName, tagName))
        print("Push tag with command:")
        print("  git push origin %s" % (tagName))




