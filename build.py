#!/usr/bin/env python
#
# Written by Johan Henriksson. Copyright (C) 2014-2017.
#
import sys
import os
import subprocess
import shutil


g_dest_path = "/usr/local"
g_verbose = False
g_exeName = "gede"

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

# Remove a file
def removeFile(filename):
    try:
        os.remove(filename)
    except OSError:
        pass

# Do a cleanup
def doClean():
    for p in ["./src",
            "./tests/tagtest",
            "./tests/highlightertest",
            "./tests/ini"
            ]:
        print("Cleaning up in %s" % (p))
        oldP = os.getcwd()
        os.chdir(p)
        if os.path.exists("Makefile"):
            if run_make(["clean"]):
                exit(1)
        else:
            os.system("rm -f *.o")
        removeFile(g_exeName)
        removeFile("Makefile")
        removeFile(".qmake.stash")
        os.chdir(oldP)

    
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


def exeExist(name):
    pathEnv = os.environ["PATH"]
    for path in pathEnv.split(":"):
        if os.path.isfile(path + "/" + name):
            return path
    return ""

def printRed(textString):
    """ Print in red text """
    CSI="\x1B["
    print(CSI+"1;31;40m" + textString + CSI + "0m")
        
def ensureExist(name):
    """ Checks if an executable exist in the PATH. """
    sys.stdout.write("Checking for " + name + "... "),
    foundPath = exeExist(name)
    if foundPath:
        print(" found in " + foundPath)
    else:
        printRed(" not found!!")

def detectQt():
    """ @brief Detects the Qt version installed in the system.
        @return The name of the qmake executable. 
    """
    sys.stdout.write("Detecting Qt version... "),
    if exeExist("qmake-qt4"):
        print("Qt4 found");
        return "qmake-qt4";
    elif exeExist("qmake-qt5"):
        print("Qt5 found");
        return "qmake-qt5";
    elif exeExist("qmake"):
        print("Qt? found (qmake)");
        return "qmake";
    print("No Qt found");
    return "qmake";


# Main entry
if __name__ == "__main__":
    try:
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
            doClean();
        if do_build:
            os.chdir("src")
            if not os.path.exists("Makefile"):
                ensureExist("make")
                ensureExist("gcc")
                ensureExist("ctags")
                qmakeName = detectQt();
                print("Generating makefile")
                if subprocess.call([qmakeName]):
                    exit(1)
            print("Compiling (please wait)")
            if run_make(["-j4"]):
                exit(1)
            os.chdir("..")
        if do_install:
            os.chdir("src")
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
                os.chmod(g_dest_path + "/bin/gede", 0o775);
            except:
                print("Failed to install files to " + g_dest_path)
                raise

            print("")
            print("Gede has been installed to " + g_dest_path + "/bin")
            print("Start it by running gede") 

    except IOError as e:
        print("I/O error({0}): {1}".format(e.errno, e.strerror))
    except SystemExit as e:
        pass
        raise e
    except:
        print("Error occured")
        raise




