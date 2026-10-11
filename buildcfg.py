#
# Written by Johan Henriksson. Copyright (C) 2024.
# Copyright (C) 2025 mebenn benny.lyons@gmx.net

from sys import platform
from build import FORCE_QT5,FORCE_QT6,AUTODETECT

#-----------------------------#
# Configuration section
g_parallel_builds = 4
g_dest_path = "/usr/local"
g_verbose = False
g_exeName = "gede"
g_qtVersionToUse = FORCE_QT5
g_qmakeQt4 = ""
g_qmakeQt5 = ""
g_qmakeQt6 = ""
g_testDirs = [
            "./tests/tagtest",
            "./tests/highlightertest",
            "./tests/ini"
            ]
g_mainSrcDir = ["./src" ]
if platform == "darwin":
    g_requiredPrograms = ["make", "clang", "ctags" ]
elif platform[:3] == "sun":
    # OpenIndiana/Illumos
    g_requiredPrograms = ["gmake", "gcc", "ctags" ]
else:
    g_requiredPrograms = ["make", "gcc", "ctags" ]
MIN_QT_VER = "4.0.0"
#-----------------------------#

