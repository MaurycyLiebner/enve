#-------------------------------------------------
#
# Project created by QtCreator 2018-12-01T10:47:47
#
#-------------------------------------------------

#QT       -= gui
QT += opengl
TARGET = core
TEMPLATE = lib

DEFINES += CORE_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /home/ailuropoda/.skia/include/images/
INCLUDEPATH += /home/ailuropoda/.skia/include/core/
INCLUDEPATH += /home/ailuropoda/.skia/include/utils/
INCLUDEPATH += /home/ailuropoda/.skia/include/gpu/
INCLUDEPATH += /home/ailuropoda/.skia/include/config/
INCLUDEPATH += /home/ailuropoda/.skia/include/ports/
INCLUDEPATH += /home/ailuropoda/.skia/include/effects/
INCLUDEPATH += /home/ailuropoda/.skia/include/private/
INCLUDEPATH += /home/ailuropoda/.skia/include/pathops/

INCLUDEPATH += /home/ailuropoda/.skia/third_party/externals/sdl/include/
INCLUDEPATH += /home/ailuropoda/.skia/third_party/vulkan/
INCLUDEPATH += /home/ailuropoda/.skia/src/gpu/

CONFIG(debug, debug|release) {
    LIBS += -L/home/ailuropoda/.skia/out/Debug
} else {
    LIBS += -L/home/ailuropoda/.skia/out/Release
    QMAKE_CFLAGS -= -O2
    QMAKE_CFLAGS -= -O1
    QMAKE_CXXFLAGS -= -O2
    QMAKE_CXXFLAGS -= -O1
    QMAKE_CFLAGS = -m64 -O3
    QMAKE_LFLAGS = -m64 -O3
    QMAKE_CXXFLAGS = -m64 -O3
}

LIBS += -lskia

SOURCES += \
    smartPointers/stdpointer.cpp \
    smartPointers/selfref.cpp \
    smartPointers/stdselfref.cpp \
    singlewidgettarget.cpp \
    singlewidgetabstraction.cpp \
    Properties/property.cpp \
    Animators/complexanimator.cpp \
    Animators/animator.cpp \
    Animators/fakecomplexanimator.cpp \
    Animators/key.cpp \
    PropertyUpdaters/propertyupdater.cpp \
    undoredo.cpp \
    Properties/propertycreator.cpp \
    exceptions.cpp \
    glhelpers.cpp \
    skia/skimagecopy.cpp \
    skia/skimagegpudraw.cpp \
    skia/skqtconversions.cpp \
    skia/AddInclude/SkGeometry.cpp \
    skia/AddInclude/SkStroke.cpp \
    skia/AddInclude/SkStrokerPriv.cpp \
    pointhelpers.cpp \
    simplemath.cpp \
    Animators/qrealpoint.cpp \
    Animators/graphanimator.cpp

HEADERS += \
        core_global.h \ 
    smartPointers/stdpointer.h \
    smartPointers/selfref.h \
    smartPointers/stdselfref.h \
    smartPointers/sharedpointerdefs.h \
    singlewidgettarget.h \
    singlewidgetabstraction.h \
    Properties/property.h \
    Animators/key.h \
    Animators/complexanimator.h \
    Animators/fakecomplexanimator.h \
    Animators/animator.h \
    PropertyUpdaters/propertyupdater.h \
    undoredo.h \
    Properties/propertycreator.h \
    exceptions.h \
    glhelpers.h \
    skia/skiadefines.h \
    skia/skiaincludes.h \
    skia/skimagecopy.h \
    skia/skimagegpudraw.h \
    skia/skqtconversions.h \
    skia/AddInclude/SkGeometry.h \
    skia/AddInclude/SkNx.h \
    skia/AddInclude/SkPaintDefaults.h \
    skia/AddInclude/SkPathPriv.h \
    skia/AddInclude/SkPointPriv.h \
    skia/AddInclude/SkStroke.h \
    skia/AddInclude/SkStrokerPriv.h \
    pointhelpers.h \
    simplemath.h \
    Animators/qrealpoint.h \
    Animators/graphanimator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
