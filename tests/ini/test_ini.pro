


lessThan(QT_MAJOR_VERSION, 5) {
    QT += gui core
}
else {
    QT += gui core widgets
}

TEMPLATE = app

SOURCES += test_ini.cpp


SOURCES+=../../src/settings.cpp ../../src/ini.cpp
HEADERS+=../../src/settings.h ../../src/ini.h

SOURCES+=../../src/log.cpp
HEADERS+=../../src/log.h
SOURCES+=../../src/util.cpp ../../src/detectdistro.cpp
HEADERS+=../../src/util.h  ../../src/detectdistro.h



QMAKE_CXXFLAGS += -I../../src  -g 


TARGET=test_ini

