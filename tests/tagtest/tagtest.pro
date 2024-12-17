


QT +=  core

TEMPLATE = app

SOURCES+=tagtest.cpp

SOURCES+=../../src/tagscanner.cpp
HEADERS+=../../src/tagscanner.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp ../../src/detectdistro.cpp
HEADERS+=../../src/util.h  ../../src/detectdistro.h

SOURCES += ../../src/rusttagscanner.cpp
HEADERS += ../../src/rusttagscanner.h

SOURCES += ../../src/adatagscanner.cpp
HEADERS += ../../src/adatagscanner.h


SOURCES += ../../src/ini.cpp ../../src/settings.cpp
HEADERS += ../../src/ini.h ../../src/settings.h

QMAKE_CXXFLAGS += -I../../src  -g


TARGET=tagtest



