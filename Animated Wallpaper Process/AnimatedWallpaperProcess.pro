QT += gui core winextras widgets multimedia multimediawidgets

LIBS += -lGdi32 -lWinmm -lRpcrt4 -luuid

VERSION = 1.0.0.0
TARGET = "AWProcess"
RC_ICONS = photos/icon.ico
QMAKE_TARGET_DESCRIPTION = "Animated Wallpaper Process"
QMAKE_TARGET_COPYRIGHT = "© 2020 INeedADollar"

HEADERS += \
    combobox.h \
    mainwindow.h \
    processthread.h \
    scrolltext.h \
    slider.h \
    switch.h \
    systemtray.h \
    utility.h

SOURCES += \
    combobox.cpp \
    main.cpp \
    mainwindow.cpp \
    processthread.cpp \
    scrolltext.cpp \
    slider.cpp \
    switch.cpp \
    systemtray.cpp \
    utility.cpp

RESOURCES += \
    resources.qrc
