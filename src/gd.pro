
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

SOURCES+=qtutil.cpp util.cpp
HEADERS+=qtutil.h util.h

SOURCES+=tree.cpp
HEADERS+=tree.h

SOURCES+=aboutdialog.cpp
HEADERS+=aboutdialog.h

SOURCES+=syntaxhighlighter.cpp syntaxhighlighterbasic.cpp syntaxhighlightercxx.cpp
SOURCES+=syntaxhighlighterrust.cpp
HEADERS+=syntaxhighlighter.h syntaxhighlighterbasic.h syntaxhighlightercxx.h
HEADERS+=syntaxhighlighterrust.h

SOURCES+=ini.cpp
HEADERS+=ini.h

SOURCES+= opendialog.cpp
HEADERS+=opendialog.h

SOURCES+=settings.cpp
HEADERS+=settings.h

SOURCES+=tagscanner.cpp tagmanager.cpp
HEADERS+=tagscanner.h   tagmanager.h

SOURCES+=rusttagscanner.cpp
HEADERS+=rusttagscanner.h

HEADERS+=config.h

SOURCES+=varctl.cpp watchvarctl.cpp autovarctl.cpp
HEADERS+=varctl.h watchvarctl.h autovarctl.h

SOURCES+=consolewidget.cpp
HEADERS+=consolewidget.h


SOURCES+=settingsdialog.cpp
HEADERS+=settingsdialog.h
FORMS += settingsdialog.ui

SOURCES+=codeviewtab.cpp
HEADERS+=codeviewtab.h
FORMS += codeviewtab.ui

SOURCES+=memorydialog.cpp memorywidget.cpp
HEADERS+=memorydialog.h memorywidget.h
FORMS += memorydialog.ui

SOURCES += processlistdialog.cpp
HEADERS += processlistdialog.h
FORMS += processlistdialog.ui

FORMS += mainwindow.ui
FORMS += aboutdialog.ui
FORMS += opendialog.ui

SOURCES+=colorbutton.cpp
HEADERS+=colorbutton.h

HEADERS+=autosignalblocker.h

RESOURCES += resource.qrc

#QMAKE_CXXFLAGS += -I./  -g

QMAKE_CXXFLAGS += -I./   -DNDEBUG



TARGET=gede



