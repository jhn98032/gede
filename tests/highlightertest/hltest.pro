QT += gui core

TEMPLATE = app

SOURCES+=hltest.cpp

SOURCES+=../../src/syntaxhighlighter.cpp
HEADERS+=../../src/syntaxhighlighter.h

SOURCES+=../../src/settings.cpp ../../src/ini.cpp
HEADERS+=../../src/settings.h ../../src/ini.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp
HEADERS+=../../src/util.h



QMAKE_CXXFLAGS += -I../../src  -g


TARGET=hltest



