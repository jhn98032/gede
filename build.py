#!/usr/bin/env python
#
# Written by Johan Henriksson. Copyright (C) 2014.
#
import sys
import os
import subprocess
import shutil


g_dest_path = "/usr/local"
g_verbose = False

# Run the make command
def run_make(a_list):
    if g_verbose:
        errcode = subprocess.call(['make'] + a_list)
    else:
        p = subprocess.Popen(['make'] + a_list, stdout=subprocess.PIPE)
        out, err = p.communicate()
        errcode = p.returncode
        if err:
            print(err)
    return errcode


# Show usage
def dump_usage():
    print("./build.py [OPTIONS]... COMMAND")
    print("where COMMAND is one of:")
    print("      install    Installs the program")
    print("      clean      Cleans the source directory")
    print("where OPTIONS are:")
    print("      --prefix=DESTDIR  The path to install to (default is %s)." % (g_dest_path))
    print("      --verbose         Verbose output.")
    print("")
    return 1



# Main entry
if __name__ == "__main__":
    try:
        os.chdir("src")
        do_clean = False
        do_install = False
        do_build = True
        for arg in sys.argv[1:]:
            if arg == "clean":
                do_build = False
                do_clean = True
            elif arg == "install":
                do_install = True
                do_build = True
            elif arg == "--help" or arg == "help":
                exit( dump_usage())
            elif arg == "--verbose":
                g_verbose = True
            elif arg.find("--prefix=") == 0:
                g_dest_path = arg[9:]
            else:
                exit(dump_usage())

        if do_clean:
            print("Cleaning")
            if os.path.exists("Makefile"):
                if run_make(["clean"]):
                    exit(1)
            else:
                os.system("rm -f *.o")
        if do_build:
            if not os.path.exists("Makefile"):
                print("Generating makefile")
                if subprocess.call(['qmake']):
                    exit(1)

            print("Compiling (please wait)")
            if run_make([]):
                exit(1)
        if do_install:
            print("Installing to '%s'" % (g_dest_path) )
            try:
                os.makedirs(g_dest_path + "/bin")
            except:
                pass
            if not os.path.isdir(g_dest_path + "/bin"):
                print("Failed to create dir")
                exit(1)
            try:
                shutil.copyfile("gede", g_dest_path + "/bin/gede")
            except:
                print("Failed to install files to " + g_dest_path)
                raise

            print("")
            print("Gede has been installed to " + g_dest_path)
            print("Start it by running gede") 

    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    except SystemExit as e:
        pass
        raise e
    except:
        print "Error occured"
        raise




