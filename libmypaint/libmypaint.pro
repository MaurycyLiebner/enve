#-------------------------------------------------
#
# Project created by QtCreator 2019-05-16T14:21:49
#
#-------------------------------------------------

QT       -= core gui

TARGET = libmypaint
TEMPLATE = lib

DEFINES += LIBMYPAINT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG(debug, debug|release) {
} else {
    QMAKE_CFLAGS -= -O2
    QMAKE_CFLAGS -= -O1
    QMAKE_CXXFLAGS -= -O2
    QMAKE_CXXFLAGS -= -O1
    QMAKE_CFLAGS = -m64 -O3
    QMAKE_LFLAGS = -m64 -O3
    QMAKE_CXXFLAGS = -m64 -O3
}

QMAKE_CXXFLAGS += -fopenmp
LIBS += -ljson-c -fopenmp

SOURCES += \
        brushmodes.cpp \
        fifo.cpp \
        helpers.cpp \
        libmypaint.cpp \
        mypaint-brush-settings.cpp \
        mypaint-brush.cpp \
        mypaint-fixed-tiled-surface.cpp \
        mypaint-mapping.cpp \
        mypaint-rectangle.cpp \
        mypaint-surface.cpp \
        mypaint-tiled-surface.cpp \
        mypaint.cpp \
        operationqueue.cpp \
        rng-double.cpp \
        tilemap.cpp

HEADERS += \
        brushmodes.h \
        brushsettings-gen.h \
        config.h \
        fifo.h \
        helpers.h \
        mypaint-brush-settings-gen.h \
        mypaint-brush-settings.h \
        mypaint-brush.h \
        mypaint-config.h \
        mypaint-fixed-tiled-surface.h \
        mypaint-glib-compat.h \
        mypaint-mapping.h \
        mypaint-rectangle.h \
        mypaint-surface.h \
        mypaint-tiled-surface.h \
        mypaint.h \
        operationqueue.h \
        rng-double.h \
        tiled-surface-private.h \
        tilemap.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    brushsettings.json
