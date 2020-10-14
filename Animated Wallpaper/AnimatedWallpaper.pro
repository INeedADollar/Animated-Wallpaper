QT += gui core winextras

greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets multimedia multimediawidgets

VERSION = 1.0.0.0
TARGET = "Animated Wallpaper"
RC_ICONS = photos/icon.ico
QMAKE_TARGET_DESCRIPTION = "Animated Wallpaper"
QMAKE_TARGET_COPYRIGHT = "© 2020 INeedADollar"

LIBS += -lOle32 -luuid

RESOURCES += \
    resources.qrc

SOURCES += \
    main.cpp \
    processthread.cpp \
    utility.cpp

HEADERS += \
    CProcessData.h \
    processthread.h \
    utility.h
