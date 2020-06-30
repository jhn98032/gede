#!/usr/bin/env python
#
# Written by Johan Henriksson. Copyright (C) 2014-2020.
#
import sys
import os
import subprocess
import shutil


g_verbose = False


# Returns the version of gede
def getGedeVersion():
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

    return (major, minor, patch)
    

# Create a debian package
def doDeb():
    verMajor,verMinor,verPatch = getGedeVersion()
    from email.Utils import formatdate
    with open("debian/changelog", "w+") as f:
        f.write("gede (%s.%s.%s) unstable; urgency=medium\n" % (verMajor,verMinor,verPatch))
        f.write("\n")
        f.write("  * See homepage\n")
        f.write("\n")
        f.write(" -- johan <johan@gede.acidron.com>  " + formatdate() + "\n")
    
    errcode = subprocess.call(['debuild','-us','-uc'])
    return errcode
    

    
# Show usage
def dump_usage():
    print("./package.py [OPTIONS]...")
    print("where OPTIONS are:")
    print("      --verbose         Verbose output.")
    print("")
    return 1



# Main entry
if __name__ == "__main__":

    try:
        doDeb()
        
    except IOError as e:
        print("I/O error({0}): {1}".format(e.errno, e.strerror))
    except SystemExit as e:
        pass
        raise e
    except:
        print("Error occured")
        raise




