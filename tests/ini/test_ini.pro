


QT +=  core

TEMPLATE = app

SOURCES += test_ini.cpp


SOURCES+=../../src/ini.cpp
HEADERS+=../../src/ini.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp ../../src/detectdistro.cpp
HEADERS+=../../src/util.h  ../../src/detectdistro.h



QMAKE_CXXFLAGS += -I../../src  -g 


TARGET=test_ini

