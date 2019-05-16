#-------------------------------------------------
#
# Project created by QtCreator 2019-05-16T14:21:49
#
#-------------------------------------------------

QT       -= core gui
LIBS += -ljson-c

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

SOURCES += \
        brushmodes.c \
        fifo.c \
        helpers.c \
        libmypaint.c \
        mypaint-brush-settings.c \
        mypaint-brush.c \
        mypaint-fixed-tiled-surface.c \
        mypaint-mapping.c \
        mypaint-rectangle.c \
        mypaint-surface.c \
        mypaint-tiled-surface.c \
        mypaint.c \
        operationqueue.c \
        rng-double.c \
        tilemap.c

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
