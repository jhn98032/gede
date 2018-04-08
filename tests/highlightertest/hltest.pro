lessThan(QT_MAJOR_VERSION, 5) {
    QT += gui core
}
else {
    QT += gui core widgets
}

TEMPLATE = app

SOURCES+=hltest.cpp

SOURCES+=../../src/syntaxhighlighter.cpp ../../src/syntaxhighlighterbasic.cpp ../../src/syntaxhighlightercxx.cpp
HEADERS+=../../src/syntaxhighlighter.h ../../src/syntaxhighlighterbasic.h ../../src/syntaxhighlightercxx.h



SOURCES+=../../src/settings.cpp ../../src/ini.cpp
HEADERS+=../../src/settings.h ../../src/ini.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp
HEADERS+=../../src/util.h



QMAKE_CXXFLAGS += -I../../src  -g


TARGET=hltest



