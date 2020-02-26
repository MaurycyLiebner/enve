# enve - 2D animations software
# Copyright (C) 2016-2020 Maurycy Liebner

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#-------------------------------------------------
#
# Project created by QtCreator 2018-12-01T10:47:47
#
#-------------------------------------------------

VERSION = 0.0.0

QT += opengl multimedia qml xml
LIBS += -lavutil -lavformat -lavcodec -lswscale -lswresample -lavresample
CONFIG += c++14
TARGET = envecore
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

ENVE_FOLDER = $$PWD/../..
THIRD_PARTY_FOLDER =  $$ENVE_FOLDER/third_party
SKIA_FOLDER = $$THIRD_PARTY_FOLDER/skia
LIBMYPAINT_FOLDER = $$THIRD_PARTY_FOLDER/libmypaint-1.5.0
QUAZIP_FOLDER = $$THIRD_PARTY_FOLDER/quazip-0.8.1

INCLUDEPATH += $$SKIA_FOLDER

INCLUDEPATH += $$LIBMYPAINT_FOLDER/include
LIBS += -L$$LIBMYPAINT_FOLDER/.libs -lmypaint -lgobject-2.0 -lglib-2.0 -ljson-c

INCLUDEPATH += $$QUAZIP_FOLDER
LIBS += -L$$QUAZIP_FOLDER/quazip -lquazip

CONFIG(debug, debug|release) {
    LIBS += -L$$SKIA_FOLDER/out/Debug -lskia
} else {
    LIBS += -L$$SKIA_FOLDER/out/Release -lskia
    QMAKE_CFLAGS -= -O2
    QMAKE_CFLAGS -= -O1
    QMAKE_CXXFLAGS -= -O2
    QMAKE_CXXFLAGS -= -O1
    QMAKE_CFLAGS = -m64 -O3
    QMAKE_LFLAGS = -m64 -O3
    QMAKE_CXXFLAGS = -m64 -O3
}

QMAKE_CXXFLAGS += -fopenmp
LIBS += -lskia -lpthread -lfreetype -lpng -ldl -fopenmp

SOURCES += \
    Animators/Expressions/expressioncomplex.cpp \
    Animators/Expressions/expressionfunction.cpp \
    Animators/Expressions/expressionfunctionbase.cpp \
    Animators/Expressions/expressionnegatefunction.cpp \
    Animators/Expressions/expressionoperator.cpp \
    Animators/Expressions/expressionparser.cpp \
    Animators/Expressions/expressionplainvalue.cpp \
    Animators/Expressions/expressionrandomfunction.cpp \
    Animators/Expressions/expressionsinglechild.cpp \
    Animators/Expressions/expressionsource.cpp \
    Animators/Expressions/expressionsourcebase.cpp \
    Animators/Expressions/expressionsourceframe.cpp \
    Animators/Expressions/expressionsourcevalue.cpp \
    Animators/Expressions/expressionvalue.cpp \
    Animators/Expressions/expressionvariable.cpp \
    Animators/SculptPath/sculptbrush.cpp \
    Animators/SculptPath/sculptnode.cpp \
    Animators/SculptPath/sculptnodebase.cpp \
    Animators/SculptPath/sculptpath.cpp \
    Animators/SculptPath/sculptpathanimator.cpp \
    Animators/SculptPath/sculptpathcollection.cpp \
    Animators/SmartPath/listofnodes.cpp \
    Animators/SmartPath/smartpath.cpp \
    Animators/SmartPath/smartpathanimatoractions.cpp \
    Animators/brushsettingsanimator.cpp \
    Animators/clampedpoint.cpp \
    Animators/clampedvalue.cpp \
    Animators/coloranimator.cpp \
    Animators/complexkey.cpp \
    Animators/customproperties.cpp \
    Animators/eboxorsound.cpp \
    Animators/eeffect.cpp \
    Animators/gradient.cpp \
    Animators/gradientpoints.cpp \
    Animators/outlinesettingsanimator.cpp \
    Animators/overlappingkeylist.cpp \
    Animators/overlappingkeys.cpp \
    Animators/paintsettingsanimator.cpp \
    Animators/qcubicsegment1danimator.cpp \
    Animators/qrealsnapshot.cpp \
    Animators/qstringanimator.cpp \
    Animators/sceneboundgradient.cpp \
    Animators/staticcomplexanimator.cpp \
    Animators/texteffectcollection.cpp \
    BlendEffects/blendeffect.cpp \
    BlendEffects/blendeffectboxshadow.cpp \
    BlendEffects/blendeffectcollection.cpp \
    BlendEffects/moveblendeffect.cpp \
    BlendEffects/targetedblendeffect.cpp \
    Boxes/animationbox.cpp \
    Boxes/boundingbox.cpp \
    Boxes/boxrendercontainer.cpp \
    Boxes/boxrenderdata.cpp \
    Boxes/boxwithpatheffects.cpp \
    Boxes/canvasrenderdata.cpp \
    Boxes/circle.cpp \
    Boxes/containerbox.cpp \
    Boxes/ecustombox.cpp \
    Boxes/effectsrenderer.cpp \
    Boxes/effectsubtaskspawner.cpp \
    Boxes/frameremapping.cpp \
    Boxes/imagebox.cpp \
    Boxes/imagerenderdata.cpp \
    Boxes/imagesequencebox.cpp \
    Boxes/internallinkbox.cpp \
    Boxes/internallinkcanvas.cpp \
    Boxes/internallinkgroupbox.cpp \
    Boxes/layerboxrenderdata.cpp \
    Boxes/linkcanvasrenderdata.cpp \
    Boxes/paintbox.cpp \
    Boxes/pathbox.cpp \
    Boxes/pathboxrenderdata.cpp \
    Boxes/patheffectsmenu.cpp \
    Boxes/rectangle.cpp \
    Boxes/renderdatahandler.cpp \
    Boxes/sculptpathbox.cpp \
    Boxes/sculptpathboxrenderdata.cpp \
    Boxes/smartvectorpath.cpp \
    Boxes/svglinkbox.cpp \
    Boxes/textbox.cpp \
    Boxes/textboxrenderdata.cpp \
    Boxes/videobox.cpp \
    CacheHandlers/cachecontainer.cpp \
    CacheHandlers/hddcachablecachehandler.cpp \
    CacheHandlers/hddcachablecont.cpp \
    CacheHandlers/hddcachablerangecont.cpp \
    CacheHandlers/imagecachecontainer.cpp \
    CacheHandlers/imagedatahandler.cpp \
    CacheHandlers/samples.cpp \
    CacheHandlers/sceneframecontainer.cpp \
    CacheHandlers/soundcachecontainer.cpp \
    CacheHandlers/soundcachehandler.cpp \
    CacheHandlers/soundtmpfilehandlers.cpp \
    CacheHandlers/tmpdeleter.cpp \
    CacheHandlers/tmploader.cpp \
    CacheHandlers/tmpsaver.cpp \
    CacheHandlers/usedrange.cpp \
    FileCacheHandlers/animationcachehandler.cpp \
    FileCacheHandlers/audiostreamsdata.cpp \
    FileCacheHandlers/filecachehandler.cpp \
    FileCacheHandlers/filedatacachehandler.cpp \
    FileCacheHandlers/filehandlerobjref.cpp \
    FileCacheHandlers/imagecachehandler.cpp \
    FileCacheHandlers/imagesequencecachehandler.cpp \
    FileCacheHandlers/soundreader.cpp \
    FileCacheHandlers/svgfilecachehandler.cpp \
    FileCacheHandlers/videocachehandler.cpp \
    FileCacheHandlers/videoframeloader.cpp \
    FileCacheHandlers/videostreamsdata.cpp \
    GUI/animationboxtopaintobjectdialog.cpp \
    GUI/boxeslistactionbutton.cpp \
    GUI/coloranimatorbutton.cpp \
    GUI/durationrectsettingsdialog.cpp \
    GUI/edialogs.cpp \
    GUI/ewidgets.cpp \
    GUI/global.cpp \
    GUI/newcanvasdialog.cpp \
    GUI/propertynamedialog.cpp \
    GUI/twocolumnlayout.cpp \
    GUI/valueinput.cpp \
    MovablePoints/gradientpoint.cpp \
    MovablePoints/pathpivot.cpp \
    Ora/oracreator.cpp \
    Ora/oraimporter.cpp \
    Ora/oraparser.cpp \
    Paint/animatedsurface.cpp \
    Paint/autotiledsurface.cpp \
    Paint/autotilesdata.cpp \
    Paint/brushcontexedwrapper.cpp \
    Paint/brushescontext.cpp \
    Paint/brushstroke.cpp \
    Paint/brushstrokeset.cpp \
    Paint/colorconversions.cpp \
    Paint/drawableautotiledsurface.cpp \
    Paint/externalpaintapphandler.cpp \
    Paint/onionskin.cpp \
    Paint/painttarget.cpp \
    Paint/simplebrushwrapper.cpp \
    Paint/tile.cpp \
    Paint/tilebitmaps.cpp \
    Paint/undoabletile.cpp \
    PathEffects/custompatheffect.cpp \
    PathEffects/dashpatheffect.cpp \
    PathEffects/displacepatheffect.cpp \
    PathEffects/duplicatepatheffect.cpp \
    PathEffects/linespatheffect.cpp \
    PathEffects/patheffectcaller.cpp \
    PathEffects/patheffectcollection.cpp \
    PathEffects/patheffectstask.cpp \
    PathEffects/solidifypatheffect.cpp \
    PathEffects/spatialdisplacepatheffect.cpp \
    PathEffects/subdividepatheffect.cpp \
    PathEffects/subpatheffect.cpp \
    PathEffects/sumpatheffect.cpp \
    PathEffects/zigzagpatheffect.cpp \
    Private/Tasks/complextask.cpp \
    Private/Tasks/gpupostprocessor.cpp \
    Private/Tasks/offscreenqgl33c.cpp \
    Private/Tasks/taskexecutor.cpp \
    Private/Tasks/taskque.cpp \
    Private/Tasks/taskquehandler.cpp \
    Private/Tasks/taskscheduler.cpp \
    Private/document.cpp \
    Private/documentrw.cpp \
    Private/esettings.cpp \
    Private/memorystructs.cpp \
    Properties/boolpropertycontainer.cpp \
    Properties/boxtargetproperty.cpp \
    Properties/emimedata.cpp \
    RasterEffects/blureffect.cpp \
    RasterEffects/customrastereffect.cpp \
    RasterEffects/rastereffect.cpp \
    RasterEffects/rastereffectcaller.cpp \
    RasterEffects/rastereffectcollection.cpp \
    RasterEffects/rastereffectmenucreator.cpp \
    RasterEffects/shadoweffect.cpp \
    ReadWrite/basicreadwrite.cpp \
    ReadWrite/ereadstream.cpp \
    ReadWrite/ewritestream.cpp \
    ReadWrite/filefooter.cpp \
    ShaderEffects/shadereffect.cpp \
    ShaderEffects/shadereffectcaller.cpp \
    ShaderEffects/shadereffectcreator.cpp \
    ShaderEffects/shadereffectprogram.cpp \
    ShaderEffects/shadervaluehandler.cpp \
    ShaderEffects/uniformspecifiercreator.cpp \
    Sound/eindependentsound.cpp \
    Sound/esound.cpp \
    Sound/esoundlink.cpp \
    Sound/esoundobjectbase.cpp \
    Sound/esoundsettings.cpp \
    Sound/evideosound.cpp \
    Sound/soundcomposition.cpp \
    Sound/soundmerger.cpp \
    Tasks/updatable.cpp \
    Timeline/animationrect.cpp \
    Timeline/durationrectangle.cpp \
    Timeline/fixedlenanimationrect.cpp \
    action.cpp \
    actions.cpp \
    canvas.cpp \
    canvashandlesmartpath.cpp \
    canvasmouseevents.cpp \
    canvasmouseinteractions.cpp \
    canvasselectedboxesactions.cpp \
    canvasselectedpointsactions.cpp \
    clipboardcontainer.cpp \
    colorhelpers.cpp \
    colorsetting.cpp \
    conncontext.cpp \
    cpurendertools.cpp \
    efiltersettings.cpp \
    etexture.cpp \
    etextureframebuffer.cpp \
    fileshandler.cpp \
    filesourcescache.cpp \
    gpurendertools.cpp \
    importhandler.cpp \
    kraimporter.cpp \
    matrixdecomposition.cpp \
    memorydatahandler.cpp \
    namefixer.cpp \
    paintsettings.cpp \
    paintsettingsapplier.cpp \
    pathoperations.cpp \
    randomgrid.cpp \
    simpletask.cpp \
    smartPointers/stdpointer.cpp \
    smartPointers/stdselfref.cpp \
    singlewidgettarget.cpp \
    Properties/property.cpp \
    Properties/comboboxproperty.cpp \
    Animators/complexanimator.cpp \
    Animators/animator.cpp \
    Animators/intanimator.cpp \
    Animators/key.cpp \
    Animators/boolanimator.cpp \
    svgimporter.cpp \
    switchablecontext.cpp \
    swt_abstraction.cpp \
    swt_rulescollection.cpp \
    texteffect.cpp \
    texteffectcaller.cpp \
    transformvalues.cpp \
    undoredo.cpp \
    exceptions.cpp \
    glhelpers.cpp \
    skia/skimagecopy.cpp \
    skia/skqtconversions.cpp \
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
    Animators/interpolationanimatort.cpp \
    nodepointvalues.cpp \
    Animators/SmartPath/smartpathcollection.cpp \
    Animators/SmartPath/smartpathkey.cpp \
    Animators/interpolationkeyt.cpp \
    Properties/boolproperty.cpp \
    PathEffects/patheffect.cpp \
    Animators/transformanimator.cpp \
    Animators/qrealanimator.cpp \
    Animators/qrealkey.cpp \
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
    MovablePoints/pathpointshandler.cpp \
    canvasbase.cpp \
    typemenu.cpp \
    zipfileloader.cpp \
    zipfilesaver.cpp

HEADERS += \
    Animators/Expressions/expressioncomplex.h \
    Animators/Expressions/expressionfunction.h \
    Animators/Expressions/expressionfunctionbase.h \
    Animators/Expressions/expressionnegatefunction.h \
    Animators/Expressions/expressionoperator.h \
    Animators/Expressions/expressionparser.h \
    Animators/Expressions/expressionplainvalue.h \
    Animators/Expressions/expressionrandomfunction.h \
    Animators/Expressions/expressionsinglechild.h \
    Animators/Expressions/expressionsource.h \
    Animators/Expressions/expressionsourcebase.h \
    Animators/Expressions/expressionsourceframe.h \
    Animators/Expressions/expressionsourcevalue.h \
    Animators/Expressions/expressionvalue.h \
    Animators/Expressions/expressionvariable.h \
    Animators/SculptPath/sculptbrush.h \
    Animators/SculptPath/sculptnode.h \
    Animators/SculptPath/sculptnodebase.h \
    Animators/SculptPath/sculptpath.h \
    Animators/SculptPath/sculptpathanimator.h \
    Animators/SculptPath/sculptpathcollection.h \
    Animators/SmartPath/listofnodes.h \
    Animators/SmartPath/smartpath.h \
    Animators/brushsettingsanimator.h \
    Animators/clampedpoint.h \
    Animators/clampedvalue.h \
    Animators/coloranimator.h \
    Animators/complexkey.h \
    Animators/customproperties.h \
    Animators/dynamiccomplexanimator.h \
    Animators/eboxorsound.h \
    Animators/eeffect.h \
    Animators/gradient.h \
    Animators/gradientpoints.h \
    Animators/interoptimalanimatort.h \
    Animators/outlinesettingsanimator.h \
    Animators/overlappingkeylist.h \
    Animators/overlappingkeys.h \
    Animators/paintsettingsanimator.h \
    Animators/qcubicsegment1danimator.h \
    Animators/qrealsnapshot.h \
    Animators/qstringanimator.h \
    Animators/sceneboundgradient.h \
    Animators/staticcomplexanimator.h \
    Animators/texteffectcollection.h \
    BlendEffects/blendeffect.h \
    BlendEffects/blendeffectboxshadow.h \
    BlendEffects/blendeffectcollection.h \
    BlendEffects/moveblendeffect.h \
    BlendEffects/targetedblendeffect.h \
    Boxes/animationbox.h \
    Boxes/boundingbox.h \
    Boxes/boxrendercontainer.h \
    Boxes/boxrenderdata.h \
    Boxes/boxwithpatheffects.h \
    Boxes/canvasrenderdata.h \
    Boxes/circle.h \
    Boxes/containerbox.h \
    Boxes/customboxcreator.h \
    Boxes/ecustombox.h \
    Boxes/effectsrenderer.h \
    Boxes/effectsubtaskspawner.h \
    Boxes/externallinkboxt.h \
    Boxes/frameremapping.h \
    Boxes/imagebox.h \
    Boxes/imagerenderdata.h \
    Boxes/imagesequencebox.h \
    Boxes/internallinkbox.h \
    Boxes/internallinkboxbase.h \
    Boxes/internallinkcanvas.h \
    Boxes/internallinkgroupbox.h \
    Boxes/layerboxrenderdata.h \
    Boxes/linkcanvasrenderdata.h \
    Boxes/paintbox.h \
    Boxes/pathbox.h \
    Boxes/pathboxrenderdata.h \
    Boxes/patheffectsmenu.h \
    Boxes/rectangle.h \
    Boxes/renderdatahandler.h \
    Boxes/sculptpathbox.h \
    Boxes/sculptpathboxrenderdata.h \
    Boxes/smartvectorpath.h \
    Boxes/svglinkbox.h \
    Boxes/textbox.h \
    Boxes/textboxrenderdata.h \
    Boxes/videobox.h \
    CacheHandlers/cachecontainer.h \
    CacheHandlers/hddcachablecachehandler.h \
    CacheHandlers/hddcachablecont.h \
    CacheHandlers/hddcachablerangecont.h \
    CacheHandlers/imagecachecontainer.h \
    CacheHandlers/imagedatahandler.h \
    CacheHandlers/samples.h \
    CacheHandlers/sceneframecontainer.h \
    CacheHandlers/soundcachecontainer.h \
    CacheHandlers/soundcachehandler.h \
    CacheHandlers/soundtmpfilehandlers.h \
    CacheHandlers/tmpdeleter.h \
    CacheHandlers/tmploader.h \
    CacheHandlers/tmpsaver.h \
    CacheHandlers/usedrange.h \
    CacheHandlers/usepointer.h \
    FileCacheHandlers/animationcachehandler.h \
    FileCacheHandlers/audiostreamsdata.h \
    FileCacheHandlers/filecachehandler.h \
    FileCacheHandlers/filedatacachehandler.h \
    FileCacheHandlers/filehandlerobjref.h \
    FileCacheHandlers/imagecachehandler.h \
    FileCacheHandlers/imagesequencecachehandler.h \
    FileCacheHandlers/soundreader.h \
    FileCacheHandlers/svgfilecachehandler.h \
    FileCacheHandlers/videocachehandler.h \
    FileCacheHandlers/videoframeloader.h \
    FileCacheHandlers/videostreamsdata.h \
    GUI/animationboxtopaintobjectdialog.h \
    GUI/boxeslistactionbutton.h \
    GUI/coloranimatorbutton.h \
    GUI/durationrectsettingsdialog.h \
    GUI/edialogs.h \
    GUI/ewidgets.h \
    GUI/global.h \
    GUI/newcanvasdialog.h \
    GUI/propertynamedialog.h \
    GUI/twocolumnlayout.h \
    GUI/valueinput.h \
    MovablePoints/gradientpoint.h \
    MovablePoints/pathpivot.h \
    Ora/oracreator.h \
    Ora/oraimporter.h \
    Ora/oraparser.h \
    Ora/orastructure.h \
    Paint/animatedsurface.h \
    Paint/autotiledsurface.h \
    Paint/autotilesdata.h \
    Paint/brushcontexedwrapper.h \
    Paint/brushescontext.h \
    Paint/brushstroke.h \
    Paint/brushstrokeset.h \
    Paint/colorconversions.h \
    Paint/drawableautotiledsurface.h \
    Paint/externalpaintapphandler.h \
    Paint/onionskin.h \
    Paint/painttarget.h \
    Paint/simplebrushwrapper.h \
    Paint/tile.h \
    Paint/tilebitmaps.h \
    Paint/undoabletile.h \
    PathEffects/custompatheffect.h \
    PathEffects/custompatheffectcreator.h \
    PathEffects/dashpatheffect.h \
    PathEffects/displacepatheffect.h \
    PathEffects/duplicatepatheffect.h \
    PathEffects/linespatheffect.h \
    PathEffects/patheffectcaller.h \
    PathEffects/patheffectcollection.h \
    PathEffects/patheffectsinclude.h \
    PathEffects/patheffectstask.h \
    PathEffects/solidifypatheffect.h \
    PathEffects/spatialdisplacepatheffect.h \
    PathEffects/subdividepatheffect.h \
    PathEffects/subpatheffect.h \
    PathEffects/sumpatheffect.h \
    PathEffects/zigzagpatheffect.h \
    Private/Tasks/complextask.h \
    Private/Tasks/gpupostprocessor.h \
    Private/Tasks/offscreenqgl33c.h \
    Private/Tasks/taskexecutor.h \
    Private/Tasks/taskque.h \
    Private/Tasks/taskquehandler.h \
    Private/Tasks/taskscheduler.h \
    Private/document.h \
    Private/esettings.h \
    Private/memorystructs.h \
    Properties/boolpropertycontainer.h \
    Properties/boxtargetproperty.h \
    Properties/emimedata.h \
    Properties/namedproperty.h \
    RasterEffects/blureffect.h \
    RasterEffects/customrastereffect.h \
    RasterEffects/rastereffect.h \
    RasterEffects/customrastereffectcreator.h \
    RasterEffects/rastereffectcaller.h \
    RasterEffects/rastereffectcollection.h \
    RasterEffects/rastereffectmenucreator.h \
    RasterEffects/rastereffectsinclude.h \
    RasterEffects/shadoweffect.h \
    ReadWrite/basicreadwrite.h \
    ReadWrite/efuturepos.h \
    ReadWrite/ereadstream.h \
    ReadWrite/ewritestream.h \
    ReadWrite/filefooter.h \
    ShaderEffects/PropertyCreators/coloranimatorcreator.h \
    ShaderEffects/PropertyCreators/intanimatorcreator.h \
    ShaderEffects/PropertyCreators/qpointfanimatorcreator.h \
    ShaderEffects/PropertyCreators/qrealanimatorcreator.h \
    ShaderEffects/PropertyCreators/shaderpropertycreator.h \
    ShaderEffects/shadereffect.h \
    ShaderEffects/shadereffectcaller.h \
    ShaderEffects/shadereffectcreator.h \
    ShaderEffects/shadereffectprogram.h \
    ShaderEffects/shadervaluehandler.h \
    ShaderEffects/uniformspecifiercreator.h \
    Sound/eindependentsound.h \
    Sound/esound.h \
    Sound/esoundlink.h \
    Sound/esoundobjectbase.h \
    Sound/esoundsettings.h \
    Sound/evideosound.h \
    Sound/soundcomposition.h \
    Sound/soundmerger.h \
    Tasks/updatable.h \
    Timeline/animationrect.h \
    Timeline/durationrectangle.h \
    Timeline/fixedlenanimationrect.h \
    action.h \
    actions.h \
    canvas.h \
    clipboardcontainer.h \
    colorhelpers.h \
    colorsetting.h \
    conncontext.h \
    conncontextobjlist.h \
    conncontextptr.h \
    core_global.h \
    cpurendertools.h \
    customhandler.h \
    customidentifier.h \
    efiltersettings.h \
    etexture.h \
    etextureframebuffer.h \
    fileshandler.h \
    filesourcescache.h \
    gpurendertools.h \
    importhandler.h \
    kraimporter.h \
    matrixdecomposition.h \
    memorydatahandler.h \
    namefixer.h \
    paintsettings.h \
    paintsettingsapplier.h \
    pathoperations.h \
    randomgrid.h \
    rangemap.h \
    regexhelpers.h \
    simpletask.h \
    smartPointers/ememory.h \
    smartPointers/eobject.h \
    smartPointers/stdpointer.h \
    smartPointers/selfref.h \
    smartPointers/stdselfref.h \
    singlewidgettarget.h \
    Properties/property.h \
    Properties/comboboxproperty.h \
    Animators/key.h \
    Animators/complexanimator.h \
    Animators/animator.h \
    Animators/intanimator.h \
    Animators/boolanimator.h \
    svgimporter.h \
    switchablecontext.h \
    swt_abstraction.h \
    swt_rulescollection.h \
    texteffect.h \
    texteffectcaller.h \
    transformvalues.h \
    undoredo.h \
    exceptions.h \
    glhelpers.h \
    skia/skiadefines.h \
    skia/skiaincludes.h \
    skia/skimagecopy.h \
    skia/skqtconversions.h \
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
    differsinterpolate.h \
    skia/skiahelpers.h \
    Animators/keyt.h \
    Animators/basedkeyt.h \
    Animators/graphkeyt.h \
    Animators/basedanimatort.h \
    Animators/graphanimatort.h \
    Animators/SmartPath/node.h \
    Animators/SmartPath/nodelist.h \
    Animators/SmartPath/smartpathanimator.h \
    Animators/interpolationanimatort.h \
    nodepointvalues.h \
    Animators/SmartPath/smartpathcollection.h \
    Animators/SmartPath/smartpathkey.h \
    Animators/interpolationkeyt.h \
    Properties/boolproperty.h \
    PathEffects/patheffect.h \
    Animators/transformanimator.h \
    Animators/qrealanimator.h \
    Animators/qrealkey.h \
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
    MovablePoints/pathpointshandler.h \
    canvasbase.h \
    typemenu.h \
    pointtypemenu.h \
    zipfileloader.h \
    zipfilesaver.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    coreresources.qrc
