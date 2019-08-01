#-------------------------------------------------
#
# Project created by QtCreator 2019-06-02T11:49:48
#
#-------------------------------------------------

QT += core qml

ENVE_FOLDER = $$PWD/../..

INCLUDEPATH += $$ENVE_FOLDER/include
DEPENDPATH += $$ENVE_FOLDER/include

INCLUDEPATH += $$ENVE_FOLDER/third_party/skia
DEPENDPATH += $$ENVE_FOLDER/third_party/skia

CONFIG(debug, debug|release) {
    ENVE_CORE_OUT = $$ENVE_FOLDER/build/Debug/src/core
} else {
    ENVE_CORE_OUT = $$ENVE_FOLDER/build/Release/src/core
    QMAKE_CFLAGS -= -O2
    QMAKE_CFLAGS -= -O1
    QMAKE_CXXFLAGS -= -O2
    QMAKE_CXXFLAGS -= -O1
    QMAKE_CFLAGS = -m64 -O3
    QMAKE_LFLAGS = -m64 -O3
    QMAKE_CXXFLAGS = -m64 -O3
}

LIBS += -L$$ENVE_CORE_OUT -lenvecore

TARGET = exampleRasterEffect
TEMPLATE = lib

DEFINES += EXAMPLERASTEREFFECT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        examplerastereffect.cpp

HEADERS += \
        examplerastereffect.h \
        examplerastereffect_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
