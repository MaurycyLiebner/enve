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
    Properties/comboboxproperty.cpp \
    Properties/intproperty.cpp \
    Animators/complexanimator.cpp \
    Animators/animator.cpp \
    Animators/fakecomplexanimator.cpp \
    Animators/key.cpp \
    PropertyUpdaters/propertyupdater.cpp \
    PropertyUpdaters/transformupdater.cpp \
    PropertyUpdaters/randomqrealgeneratorupdater.cpp \
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
    Animators/graphanimator.cpp \
    Animators/graphkey.cpp \
    Animators/interpolationkey.cpp \
    Animators/interpolationanimator.cpp \
    framerange.cpp \
    Segments/quadsegment.cpp \
    Segments/conicsegment.cpp \
    Segments/cubiclist.cpp \
    Segments/cubicnode.cpp \
    Segments/qcubicsegment2d.cpp \
    Segments/qcubicsegment1d.cpp \
    Animators/animatort.cpp \
    Animators/interpolatedanimator.cpp \
    Animators/steppedanimator.cpp \
    basicreadwrite.cpp \
    differsinterpolate.cpp \
    skia/skiahelpers.cpp \
    Animators/keyt.cpp \
    Animators/basedkeyt.cpp \
    Animators/graphkeyt.cpp \
    Animators/basedanimatort.cpp \
    Animators/graphanimatort.cpp \
    Animators/SmartPath/node.cpp \
    Animators/SmartPath/nodelist.cpp \
    Animators/SmartPath/smartpathanimator.cpp \
    Animators/SmartPath/smartpathcontainer.cpp \
    Animators/interpolationanimatort.cpp \
    nodepointvalues.cpp \
    Animators/SmartPath/smartpathcollection.cpp \
    Animators/SmartPath/smartpathkey.cpp \
    Animators/interpolationkeyt.cpp \
    Properties/boolproperty.cpp \
    PathEffects/patheffect.cpp \
    Animators/polylineanimator.cpp \
    Animators/transformanimator.cpp \
    Animators/qrealanimator.cpp \
    Animators/qrealkey.cpp \
    Animators/randomqrealgenerator.cpp \
    Animators/qrealvalueeffect.cpp \
    Animators/qpointfanimator.cpp \
    MovablePoints/movablepoint.cpp \
    MovablePoints/nonanimatedmovablepoint.cpp \
    MovablePoints/animatedpoint.cpp \
    MovablePoints/boxpathpoint.cpp \
    MovablePoints/smartnodepoint.cpp \
    MovablePoints/segment.cpp \
    MovablePoints/smartctrlpoint.cpp \
    MovablePoints/pointshandler.cpp \
    MovablePoints/pathpointshandler.cpp

HEADERS += \
    core_global.h \
    smartPointers/stdpointer.h \
    smartPointers/selfref.h \
    smartPointers/stdselfref.h \
    smartPointers/sharedpointerdefs.h \
    singlewidgettarget.h \
    singlewidgetabstraction.h \
    Properties/property.h \
    Properties/comboboxproperty.h \
    Properties/intproperty.h \
    Animators/key.h \
    Animators/complexanimator.h \
    Animators/fakecomplexanimator.h \
    Animators/animator.h \
    PropertyUpdaters/propertyupdater.h \
    PropertyUpdaters/transformupdater.h \
    PropertyUpdaters/randomqrealgeneratorupdater.h \
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
    Animators/graphanimator.h \
    Animators/graphkey.h \
    Animators/interpolationkey.h \
    Animators/interpolationanimator.h \
    framerange.h \
    Segments/quadsegment.h \
    Segments/conicsegment.h \
    Segments/cubiclist.h \
    Segments/cubicnode.h \
    Segments/qcubicsegment2d.h \
    Segments/qcubicsegment1d.h \
    Animators/animatort.h \
    Animators/interpolatedanimator.h \
    Animators/steppedanimator.h \
    basicreadwrite.h \
    differsinterpolate.h \
    castmacros.h \
    skia/skiahelpers.h \
    Animators/keyt.h \
    Animators/basedkeyt.h \
    Animators/graphkeyt.h \
    Animators/basedanimatort.h \
    Animators/graphanimatort.h \
    Animators/SmartPath/node.h \
    Animators/SmartPath/nodelist.h \
    Animators/SmartPath/smartpathanimator.h \
    Animators/SmartPath/smartpathcontainer.h \
    Animators/interpolationanimatort.h \
    nodepointvalues.h \
    Animators/SmartPath/smartpathcollection.h \
    Animators/SmartPath/smartpathkey.h \
    Animators/interpolationkeyt.h \
    Properties/boolproperty.h \
    PathEffects/patheffect.h \
    Animators/polylineanimator.h \
    Animators/transformanimator.h \
    Animators/qrealanimator.h \
    Animators/qrealkey.h \
    Animators/randomqrealgenerator.h \
    Animators/qrealvalueeffect.h \
    Animators/qpointfanimator.h \
    MovablePoints/movablepoint.h \
    MovablePoints/nonanimatedmovablepoint.h \
    MovablePoints/animatedpoint.h \
    MovablePoints/boxpathpoint.h \
    MovablePoints/smartnodepoint.h \
    MovablePoints/segment.h \
    MovablePoints/smartctrlpoint.h \
    MovablePoints/pointshandler.h \
    MovablePoints/pathpointshandler.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
