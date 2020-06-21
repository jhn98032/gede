#!/usr/bin/env python
#
# Written by Johan Henriksson. Copyright (C) 2014-2017.
#
import sys
import os
import subprocess
import shutil


FORCE_QT4=1
FORCE_QT5=2
AUTODETECT=3

g_dest_path = "/usr/local"
g_verbose = False
g_exeName = "gede"
g_qtVersionToUse = AUTODETECT
g_qmakeQt4 = ""
g_qmakeQt5 = ""

        
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
    print("      --use-qt4         Use qt4")
    print("      --use-qt5         Use qt5")
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
    global g_qmakeQt4
    global g_qmakeQt5
    sys.stdout.write("Detecting Qt version... "),
    qtVerList = []
    if exeExist("qmake-qt4"):
        qtVerList += ["Qt4"]
        g_qmakeQt4 = "qmake-qt4";
    if exeExist("qmake-qt5"):
        qtVerList += ["Qt5"]
        g_qmakeQt5 = "qmake-qt5";
    if exeExist("qmake"):
        qtVerList += ["Qt?"]
        if not g_qmakeQt5:
            g_qmakeQt5 = "qmake";
        else:
            g_qmakeQt4 = "qmake";
    sys.stdout.write(", ".join(qtVerList) + "\n")
    if (not g_qmakeQt4) and (not g_qmakeQt5):
        print("No Qt found");

    # Which version to use?
    qmakeName = "qmake"
    if g_qtVersionToUse == FORCE_QT4 and g_qmakeQt4:
        qmakeName = g_qmakeQt4;
    elif g_qmakeQt5:
        qmakeName = g_qmakeQt5;
    elif g_qmakeQt4:
        qmakeName = g_qmakeQt4;
    print("Using '" + qmakeName + "'")
    sys.stdout.flush()

    return qmakeName;


# Main entry
if __name__ == "__main__":
    try:
        do_clean = False
        do_install = False
        do_build = True
        do_buildAll = False
        
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
            elif arg == "--buildall":
                do_buildAll = True
            elif arg.find("--prefix=") == 0:
                g_dest_path = arg[9:]
            elif arg == "--use-qt4":
                g_qtVersionToUse = FORCE_QT4
            elif arg == "--use-qt5":
                g_qtVersionToUse = FORCE_QT5
            else:
                exit(dump_usage())

        if do_clean:
            doClean();
        if do_build:
            ensureExist("make")
            ensureExist("gcc")
            ensureExist("ctags")
            sys.stdout.flush()
            
            olddir = os.getcwd()
            if do_buildAll:
                srcDirList = ["src", "tests/tagtest", "tests/highlightertest"]
            else:
                srcDirList = ["src"]
            for srcdir in srcDirList:
                os.chdir(srcdir)
                if not os.path.exists("Makefile"):
                    qmakeName = detectQt();
                    print("Generating makefile")
                    sys.stdout.flush()
                    if subprocess.call([qmakeName]):
                        exit(1)
                print("Compiling in " + srcdir + " (please wait)")
                if run_make(["-j4"]):
                    exit(1)
                os.chdir(olddir)
                
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




