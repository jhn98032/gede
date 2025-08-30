#!/usr/bin/env python3
#
# Written by Johan Henriksson. Copyright (C) 2014-2024.
# Copyright (C) 1.6.2025 by benny.lyons@gmx.net
#
import sys
import os
import subprocess
import shutil
import glob
from sys import platform


FORCE_QT4=1
FORCE_QT5=2
FORCE_QT6=3
AUTODETECT=4

import buildcfg



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
    p = subprocess.Popen([qmakeExe, "--version"], stdout=subprocess.PIPE, universal_newlines=True)
    out, err = p.communicate()
    errcode = p.returncode
    if not err:
        outRows = out.split('\n')
        for row in outRows:
            if row.startswith("Using Qt version "):
                verStr = row.split(' ')[3]
    return verStr
    
        
# Run a command
def run_program(program_name, a_list, verbose = False):
    if verbose:
        errcode = subprocess.call([program_name] + a_list)
    else:
        p = subprocess.Popen([program_name] + a_list, stdout=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        errcode = p.returncode
        if err:
            print(err)
    return errcode

# Run make
def run_make(a_list, verbose = False):
    if platform[:3] == 'sun':
        return run_program("gmake", a_list, verbose)
    else:
        return run_program("make", a_list, verbose)

# Remove a file
def removeFile(filename):
    try:
        if buildcfg.g_verbose:
            print("rm " + filename)
        os.remove(filename)
    except OSError:
        pass

# Do a cleanup
def doClean():
    for p in buildcfg.g_testDirs + buildcfg.g_mainSrcDir:
        print("Cleaning up in %s" % (p))
        oldP = os.getcwd()
        os.chdir(p)
        if os.path.exists("Makefile"):
            if run_make(["clean"], False):
                exit(1)
        else:
            os.system("rm -f *.o")
        removeFile(buildcfg.g_exeName)
        removeFile("Makefile")
        removeFile(".qmake.stash")
        if platform == "darwin" and os.path.exists(g_exeName + ".app"):
            shutil.rmtree(g_exeName + ".app")
        os.chdir(oldP)


# Show usage
def dump_usage():
    print("./build.py [OPTIONS]... COMMAND")
    print("where COMMAND is one of:")
    print("      install    Installs the program")
    print("      clean      Cleans the source directory")
    print("where OPTIONS are:")
    print("      --prefix=DESTDIR  The path to install to (default is %s)." % (buildcfg.g_dest_path))
    print("      --verbose         Verbose output.")
    print("      --use-qt4         Use qt4")
    print("      --use-qt5         Use qt5")
    print("      --use-qt6         Use qt6")
    print("      --build-all       Build test programs also")
    print("      --parallel-builds=NUM   Number of parallel jobs to run in parallel")
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
        p = subprocess.Popen([qmakeExe, "-query", "QT_VERSION"], stdout=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        errcode = p.returncode
        if err:
            print(err)
        else:
            verStr = out.strip()
            print(verStr)
    except Exception as e:
        print("Error occured when started qmake: " + str(e))
        raise
    return verStr

def detectQt():
    """ @brief Detects the Qt version installed in the system.
        @return The name of the qmake executable. 
    """
    sys.stdout.write("Detecting Qt version... ")
    qtVerList = []
    if exeExist("qmake-qt4"):
        qtVerList += ["Qt4 (qmake-qt4)"]
        buildcfg.g_qmakeQt4 = "qmake-qt4";
    if exeExist("qmake-qt5"):
        qtVerList += ["Qt5 (qmake-qt5)"]
        buildcfg.g_qmakeQt5 = "qmake-qt5";
    if exeExist("qmake6"):
        qtVerList += ["Qt6 (qmake6)"]
        buildcfg.g_qmakeQt6 = "qmake6";
    if exeExist("qmake"):
        ver = detectQmakeQtVer("qmake")[0]
        if ver == "6":
            buildcfg.g_qmakeQt6 = "qmake";
            qtVerList += ["Qt6 (qmake)"]
        elif ver == "5":
            buildcfg.g_qmakeQt5 = "qmake";
            qtVerList += ["Qt5 (qmake)"]
        elif ver == "4":
            buildcfg.g_qmakeQt4 = "qmake";
            qtVerList += ["Qt4 (qmake)"]
        else:
            buildcfg.g_qmakeQt5 = "qmake";
            qtVerList += ["Qt? (qmake)"]
    sys.stdout.write(", ".join(qtVerList) + "\n")
    if (not buildcfg.g_qmakeQt4) and (not buildcfg.g_qmakeQt5) and (not buildcfg.g_qmakeQt6):
        print("No Qt found");

    # Which version to use?
    qmakeName = ""
    if buildcfg.g_qtVersionToUse == FORCE_QT4:
        os.environ["QT_SELECT"] = "qt4"
        if buildcfg.g_qmakeQt4:
            qmakeName = buildcfg.g_qmakeQt4;
        else:
            raise RuntimeError("Failed to find qt4 qmake")
    elif buildcfg.g_qtVersionToUse == FORCE_QT5:
        os.environ["QT_SELECT"] = "qt5"
        if buildcfg.g_qmakeQt5:
            qmakeName = buildcfg.g_qmakeQt5;
        else:
            raise RuntimeError("Failed to find qt4 qmake")
    elif buildcfg.g_qtVersionToUse == FORCE_QT6:
        os.environ["QT_SELECT"] = "qt6"
        if buildcfg.g_qmakeQt6:
            qmakeName = buildcfg.g_qmakeQt6;
        else:
            raise RuntimeError("Failed to find qt6 qmake")
    elif buildcfg.g_qmakeQt6:
        qmakeName = buildcfg.g_qmakeQt6;
    elif buildcfg.g_qmakeQt5:
        qmakeName = buildcfg.g_qmakeQt5;
    elif buildcfg.g_qmakeQt4:
        qmakeName = buildcfg.g_qmakeQt4;
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
                buildcfg.g_verbose = True
            elif arg == "--build-all":
                do_buildAll = True
            elif arg.find("--prefix=") == 0:
                buildcfg.g_dest_path = arg[9:]
            elif arg == "--use-qt4":
                buildcfg.g_qtVersionToUse = FORCE_QT4
            elif arg == "--use-qt5":
                buildcfg.g_qtVersionToUse = FORCE_QT5
            elif arg == "--use-qt6":
                buildcfg.g_qtVersionToUse = FORCE_QT6
            elif arg.startswith("--parallel-builds="):
                buildcfg.g_parallel_builds = int(arg[18:])
            else:
                exit(dump_usage())

        if do_clean:
            doClean();
        if do_build:
            for reqPrg in buildcfg.g_requiredPrograms:
                ensureExist(reqPrg)
            sys.stdout.flush()
            
            olddir = os.getcwd()
            if do_buildAll:
                srcDirList = buildcfg.g_mainSrcDir + buildcfg.g_testDirs 
            else:
                srcDirList = buildcfg.g_mainSrcDir
            for srcdir in srcDirList:
                os.chdir(srcdir)
                if not os.path.exists("Makefile"):
                    [qmakeName, qtVer] = detectQt();
                    if not qmakeName:
                        raise RuntimeError("Unable to find qmake")
                    if verifyVersion(qtVer, buildcfg.MIN_QT_VER):
                        raise RuntimeError("Unable to find Qt >=" + MIN_QT_VER)
                    else:
                
                        print("Generating makefile")
                        sys.stdout.flush()
                        if buildcfg.g_verbose:
                            qmakeCallCmds = [qmakeName]
                        else:
                            qmakeCallCmds = [qmakeName, "CONFIG+=silent"]
                        if subprocess.call(qmakeCallCmds):
                            raise RuntimeError("Running qmake failed")
                        print("Cleaning up in " + srcdir + " (please wait)")
                        run_make(["clean"], True)
                print("Compiling in " + srcdir + " (please wait)")
                if run_make(["-j%d" % (buildcfg.g_parallel_builds)], True):
                    raise RuntimeError("Make failed")
                os.chdir(olddir)

            if do_buildAll:
                # Loop through all tests
                for srcdir in buildcfg.g_testDirs:
                    this_dir = os.getcwd()
                    os.chdir(srcdir)

                    # Get exe name
                    exe_name = "./" + os.path.basename(srcdir)
                    if not os.path.exists(exe_name):
                        pro_files = glob.glob("*.pro")
                        if pro_files:
                            exe_name, _ = os.path.splitext(pro_files[0])

                    # Execute tests
                    if os.path.exists(exe_name):
                        print("Executing '%s'" % (exe_name))
                        if run_program("./" + exe_name,[], buildcfg.g_verbose):
                            raise RuntimeError("Test %s failed" % (exe_name))
                    os.chdir(this_dir)
            
        if do_install:
            os.chdir("src")
            print("Installing to '%s'" % (buildcfg.g_dest_path) )

            # Create destination path
            try:
                os.makedirs(buildcfg.g_dest_path + "/bin")
            except:
                pass
            if not os.path.isdir(buildcfg.g_dest_path + "/bin"):
                raise RuntimeError("Failed to create dir")

            # Copy to destination path
            try:
                if platform == "darwin":
                    shutil.copy("%s.app/Contents/MacOS/%s" % (buildcfg.g_exeName,buildcfg.g_exeName), buildcfg.g_dest_path + "/bin")
                    shutil.copytree("%s.app" % (buildcfg.g_exeName), "/Applications/%s.app" % (buildcfg.g_exeName), dirs_exist_ok=True)
                else:
                    shutil.copyfile(buildcfg.g_exeName, buildcfg.g_dest_path + "/bin/" + buildcfg.g_exeName)
                    os.chmod(buildcfg.g_dest_path + "/bin/" + buildcfg.g_exeName, 0o775);
            except:
                raise RuntimeError("Failed to install files to " + buildcfg.g_dest_path)

            print("")
            print(buildcfg.g_exeName + " has been installed to " + buildcfg.g_dest_path + "/bin")

    except RuntimeError as e:
        print("Runtime error: {0}".format(str(e)))
        exit(1)
    except IOError as e:
        print("I/O error({0}): {1}".format(e.errno, str(e)))
        exit(1)
    except SystemExit as e:
        pass
        raise e
    except:
        print("Error occured")
        raise




