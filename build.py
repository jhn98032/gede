#!/usr/bin/env python3
#
# Written by Johan Henriksson. Copyright (C) 2014-2021.
#
import sys
import os
import subprocess
import shutil


FORCE_QT4=1
FORCE_QT5=2
AUTODETECT=3

#-----------------------------#
# Configuration section
g_dest_path = "/usr/local"
g_verbose = False
g_exeName = "gede"
g_qtVersionToUse = FORCE_QT5
g_qmakeQt4 = ""
g_qmakeQt5 = ""
g_allSrcDirs = ["./src",
            "./tests/tagtest",
            "./tests/highlightertest",
            "./tests/ini"
            ]
g_mainSrcDir = ["./src" ]
g_requiredPrograms = ["make", "gcc", "ctags" ]
MIN_QT_VER = "4.0.0"
#-----------------------------#


# Check if Qt version is new enough.
def verifyVersion(qtVer, minQtVer):
    if len(qtVer.split(".")) != 3:
        return -1
    [f1,f2,f3] = qtVer.split(".")
    [m1,m2,m3] = minQtVer.split(".")
    if f1 < m1:
        return 1
    elif f1 == m1:
        if f2 < m2:
            return 1
        elif f2 == m2:
            if f3 < m3:
                return 1
    
    return 0



# Detect which version of Qt that a qmake executable will use
def detectQmakeQtVer(qmakeExe):
    verStr = "?"
    p = subprocess.Popen([qmakeExe, "--version"], stdout=subprocess.PIPE, text=True)
    out, err = p.communicate()
    errcode = p.returncode
    if not err:
        outRows = out.split('\n')
        for row in outRows:
            if row.startswith("Using Qt version "):
                verStr = row.split(' ')[3]
    return verStr
    
        
# Run the make command
def run_make(a_list):
    if g_verbose:
        errcode = subprocess.call(['make'] + a_list)
    else:
        p = subprocess.Popen(['make'] + a_list, stdout=subprocess.PIPE, text=True)
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
    for p in g_allSrcDirs:
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
    print("      --build-all       Build test programs also")
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

# Return the version of Qt. Eg: getQtVersion("qmake") => "5.1.2".
def getQtVersion(qmakeExe):
    verStr = "?"
    # Query qt version
    print("Qt version:", end=' ')
    try:
        p = subprocess.Popen([qmakeExe, "-query", "QT_VERSION"], stdout=subprocess.PIPE, text=True)
        out, err = p.communicate()
        errcode = p.returncode
        if err:
            print(err)
        else:
            verStr = out.strip()
            print(verStr)
    except Exception as e:
        print("Error occured: " + str(e))
        raise
    return verStr

def detectQt():
    """ @brief Detects the Qt version installed in the system.
        @return The name of the qmake executable. 
    """
    global g_qmakeQt4
    global g_qmakeQt5
    sys.stdout.write("Detecting Qt version... ")
    qtVerList = []
    if exeExist("qmake-qt4"):
        qtVerList += ["Qt4 (qmake-qt4)"]
        g_qmakeQt4 = "qmake-qt4";
    if exeExist("qmake-qt5"):
        qtVerList += ["Qt5 (qmake-qt5)"]
        g_qmakeQt5 = "qmake-qt5";
    if exeExist("qmake"):
        ver = detectQmakeQtVer("qmake")[0]
        if ver == "5":
            g_qmakeQt5 = "qmake";
            qtVerList += ["Qt5 (qmake)"]
        elif ver == "4":
            g_qmakeQt4 = "qmake";
            qtVerList += ["Qt4 (qmake)"]
        else:
            g_qmakeQt5 = "qmake";
            qtVerList += ["Qt? (qmake)"]
    sys.stdout.write(", ".join(qtVerList) + "\n")
    if (not g_qmakeQt4) and (not g_qmakeQt5):
        print("No Qt found");

    # Which version to use?
    qmakeName = ""
    if g_qtVersionToUse == FORCE_QT4:
        if g_qmakeQt4:
            qmakeName = g_qmakeQt4;
    elif g_qtVersionToUse == FORCE_QT5:
        if g_qmakeQt5:
            qmakeName = g_qmakeQt5;
    elif g_qmakeQt5:
        qmakeName = g_qmakeQt5;
    elif g_qmakeQt4:
        qmakeName = g_qmakeQt4;
    if qmakeName:
        print("Using '" + qmakeName + "'")
    else:
        print("Failed to find suitable qmake")
    sys.stdout.flush()

    verStr = getQtVersion(qmakeName)
    return [qmakeName, verStr];


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
            elif arg == "--build-all":
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
            for reqPrg in g_requiredPrograms:
                ensureExist(reqPrg)
            sys.stdout.flush()
            
            olddir = os.getcwd()
            if do_buildAll:
                srcDirList = g_allSrcDirs
            else:
                srcDirList = g_mainSrcDir
            for srcdir in srcDirList:
                os.chdir(srcdir)
                if not os.path.exists("Makefile"):
                    [qmakeName, qtVer] = detectQt();
                    if not qmakeName:
                        exit(1)
                    if verifyVersion(qtVer, MIN_QT_VER):
                        print("Unable to find Qt >=" + MIN_QT_VER)
                    else:
                
                        print("Generating makefile")
                        sys.stdout.flush()
                        if subprocess.call([qmakeName]):
                            exit(1)
                        print("Cleaning up in " + srcdir + " (please wait)")
                        run_make(["clean"])
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
                shutil.copyfile(g_exeName, g_dest_path + "/bin/" + g_exeName)
                os.chmod(g_dest_path + "/bin/" + g_exeName, 0o775);
            except:
                print("Failed to install files to " + g_dest_path)
                raise

            print("")
            print(g_exeName + " has been installed to " + g_dest_path + "/bin")

    except IOError as e:
        print("I/O error({0}): {1}".format(e.errno, e.strerror))
    except SystemExit as e:
        pass
        raise e
    except:
        print("Error occured")
        raise




