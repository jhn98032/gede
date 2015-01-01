QT += gui core

TEMPLATE = app

SOURCES+=tagtest.cpp

SOURCES+=../../src/tagscanner.cpp
HEADERS+=../../src/tagscanner.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp
HEADERS+=../../src/util.h



QMAKE_CXXFLAGS += -I../../src  -g


TARGET=tagtest



