QT +=  core

TEMPLATE = app

SOURCES+=hltest.cpp

SOURCES+=../../src/syntaxhighlighter.cpp ../../src/syntaxhighlighterbasic.cpp ../../src/syntaxhighlightercxx.cpp
SOURCES+=../../src/syntaxhighlighterrust.cpp
HEADERS+=../../src/syntaxhighlighter.h ../../src/syntaxhighlighterbasic.h ../../src/syntaxhighlightercxx.h
HEADERS+=../../src/syntaxhighlighterrust.h

SOURCES+=../../src/syntaxhighlighterfortran.cpp
HEADERS+=../../src/syntaxhighlighterfortran.h

SOURCES+=../../src/parsecharqueue.cpp
HEADERS+=../../src/parsecharqueue.h


SOURCES+=../../src/settings.cpp ../../src/ini.cpp
HEADERS+=../../src/settings.h ../../src/ini.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp ../../src/detectdistro.cpp
HEADERS+=../../src/util.h  ../../src/detectdistro.h



QMAKE_CXXFLAGS += -I../../src  -g


TARGET=hltest



