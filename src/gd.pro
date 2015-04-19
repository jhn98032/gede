
lessThan(QT_MAJOR_VERSION, 5) {
    QT += gui core
}
else {
    QT += gui core widgets
}


TEMPLATE = app

SOURCES+=gd.cpp

SOURCES+=mainwindow.cpp
HEADERS+=mainwindow.h

SOURCES+=codeview.cpp
HEADERS+=codeview.h

SOURCES+=gdbmiparser.cpp core.cpp
HEADERS+=gdbmiparser.h core.h

SOURCES+=com.cpp
HEADERS+=com.h

SOURCES+=log.cpp
HEADERS+=log.h

SOURCES+=util.cpp
HEADERS+=util.h

SOURCES+=tree.cpp
HEADERS+=tree.h

SOURCES+=aboutdialog.cpp
HEADERS+=aboutdialog.h

SOURCES+=syntaxhighlighter.cpp
HEADERS+=syntaxhighlighter.h

SOURCES+=ini.cpp
HEADERS+=ini.h

SOURCES+= opendialog.cpp
HEADERS+=opendialog.h

SOURCES+=settings.cpp
HEADERS+=settings.h

SOURCES+=tagscanner.cpp
HEADERS+=tagscanner.h

HEADERS+=config.h

SOURCES+=varctl.cpp watchvarctl.cpp autovarctl.cpp
HEADERS+=varctl.h watchvarctl.h autovarctl.h

SOURCES+=settingsdialog.cpp
HEADERS+=settingsdialog.h
FORMS += settingsdialog.ui

SOURCES+=codeviewtab.cpp
HEADERS+=codeviewtab.h
FORMS += codeviewtab.ui


FORMS += mainwindow.ui
FORMS += aboutdialog.ui
FORMS += opendialog.ui

RESOURCES += resource.qrc

#QMAKE_CXXFLAGS += -I./  -g

QMAKE_CXXFLAGS += -I./   -DNDEBUG



TARGET=gede



