#-------------------------------------------------
#
# Project created by QtCreator 2016-08-06T14:25:20
#
#-------------------------------------------------

QT += multimedia core gui svg opengl sql qml xml concurrent #widgets-private # gui-private core-private
LIBS += -lavutil -lavformat -lavcodec -lswscale -lswresample -lavresample -ltcmalloc -lmypaint-2.0

QMAKE_RPATHDIR += ../core/
LIBS += -L$$OUT_PWD/../core/ -lcore
INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

INCLUDEPATH += /usr/local/include/libmypaint-2.0/

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

QMAKE_CXXFLAGS += -fopenmp
LIBS += -lskia -lpthread -lfreetype -lpng -ldl -lSDL2 -lSDL2_image \
        -lSDL2_ttf -lfontconfig -fopenmp# -lX11


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AniVect
TEMPLATE = app

SOURCES += main.cpp\
    canvas.cpp \
    canvasmouseinteractions.cpp \
    GUI/ColorWidgets/colorlabel.cpp \
    GUI/ColorWidgets/colorsettingswidget.cpp \
    GUI/ColorWidgets/colorvaluerect.cpp \
    GUI/ColorWidgets/colorvaluespin.cpp \
    GUI/ColorWidgets/colorwidget.cpp \
    GUI/ColorWidgets/h_wheel_sv_triangle.cpp \
    GUI/ColorWidgets/colorpickingwidget.cpp \
    GUI/ColorWidgets/savedcolorbutton.cpp \
    GUI/ColorWidgets/savedcolorswidget.cpp \
    GUI/ColorWidgets/savedcolorwidgets.cpp \
    GUI/GradientWidgets/gradientwidget.cpp \
    GUI/ColorWidgets/glwidget.cpp \
    svgimporter.cpp \
    Animators/transformanimator.cpp \
    Animators/boolanimator.cpp \
    Animators/coloranimator.cpp \
    Animators/qstringanimator.cpp \
    Animators/qpointfanimator.cpp \
    Boxes/circle.cpp \
    Boxes/textbox.cpp \
    Boxes/rectangle.cpp \
    Boxes/imagebox.cpp \
    Animators/intanimator.cpp \
    Animators/qrealanimator.cpp \
    Boxes/pathbox.cpp \
    Boxes/boundingbox.cpp \
    Boxes/boxesgroup.cpp \
    Animators/effectanimators.cpp \
    Boxes/animationbox.cpp \
    pathoperations.cpp \
    Boxes/linkbox.cpp \
    Boxes/boxpainthandler.cpp \
    PixmapEffects/pixmapeffect.cpp \
    Boxes/particlebox.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollarea.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollwidget.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.cpp \
    GUI/BoxesList/OptimalScrollArea/singlewidget.cpp \
    GUI/BoxesList/boxsinglewidget.cpp \
    GUI/BoxesList/boxscrollwidgetvisiblepart.cpp \
    GUI/BoxesList/boxscrollwidget.cpp \
    GUI/BoxesList/boxeslistactionbutton.cpp \
    canvasselectedpointsactions.cpp \
    canvasselectedboxesactions.cpp \
    clipboardcontainer.cpp \
    Animators/paintsettings.cpp \
    Boxes/videobox.cpp \
    Sound/singlesound.cpp \
    Sound/soundcomposition.cpp \
    Boxes/boundingboxrendercontainer.cpp \
    durationrectangle.cpp \
    GUI/BoxesList/boolpropertywidget.cpp \
    Boxes/imagesequencebox.cpp \
    Boxes/rendercachehandler.cpp \
    memorychecker.cpp \
    memoryhandler.cpp \
    outputgenerator.cpp \
    GUI/RenderWidgets/renderwidget.cpp \
    GUI/RenderWidgets/renderinstancewidget.cpp \
    renderinstancesettings.cpp \
    GUI/GradientWidgets/gradientslistwidget.cpp \
    GUI/GradientWidgets/displayedgradientswidget.cpp \
    GUI/GradientWidgets/currentgradientwidget.cpp \
    filesourcescache.cpp \
    GUI/RenderWidgets/closablecontainer.cpp \
    GUI/RenderWidgets/renderdestinationdialog.cpp \
    PathEffects/patheffectanimators.cpp \
    GUI/BoxesList/coloranimatorbutton.cpp \
    updatable.cpp \
    GUI/BoxesList/boxtargetwidget.cpp \
    Properties/boxtargetproperty.cpp \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.cpp \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.cpp \
    Paint/brush.cpp \
    Paint/surface.cpp \
    Paint/tile.cpp \
    Boxes/paintbox.cpp \
    Paint/animatedsurface.cpp \
    avfileio.cpp \
    GUI/paintboxsettingsdialog.cpp \
    execdelegator.cpp \
    Paint/tilesdata.cpp \
    GUI/BoxesList/boxscrollarea.cpp \
    Properties/comboboxproperty.cpp \
    Animators/randomqrealgenerator.cpp \
    Animators/qrealvalueeffect.cpp \
    Properties/intproperty.cpp \
    windowsinglewidgettarget.cpp \
    Boxes/bone.cpp \
    drawpath.cpp \
    videoencoder.cpp \
    GUI/RenderWidgets/rendersettingsdialog.cpp \
    GUI/RenderWidgets/outputsettingsprofilesdialog.cpp \
    GUI/RenderWidgets/outputsettingsdisplaywidget.cpp \
    Boxes/renderdatahandler.cpp \
    Boxes/boundingboxrenderdata.cpp \
    application.cpp \
    PathEffects/displacepatheffect.cpp \
    PathEffects/lengthpatheffect.cpp \
    PathEffects/duplicatepatheffect.cpp \
    PathEffects/solidifypatheffect.cpp \
    PathEffects/operationpatheffect.cpp \
    PixmapEffects/blureffect.cpp \
    PixmapEffects/shadoweffect.cpp \
    PixmapEffects/lineseffect.cpp \
    PixmapEffects/circleseffect.cpp \
    PixmapEffects/swirleffect.cpp \
    PixmapEffects/oileffect.cpp \
    PixmapEffects/implodeeffect.cpp \
    PixmapEffects/desaturateeffect.cpp \
    PixmapEffects/colorizeeffect.cpp \
    PixmapEffects/replacecoloreffect.cpp \
    PixmapEffects/contrasteffect.cpp \
    PixmapEffects/brightnesseffect.cpp \
    PixmapEffects/sampledmotionblureffect.cpp \
    GUI/boxeslistkeysviewwidget.cpp \
    GUI/boxeslistanimationdockwidget.cpp \
    GUI/actionbutton.cpp \
    GUI/animationdockwidget.cpp \
    GUI/animationwidgetscrollbar.cpp \
    GUI/canvaswindow.cpp \
    GUI/graphboxeslist.cpp \
    GUI/keysview.cpp \
    GUI/mainwindow.cpp \
    GUI/newcanvasdialog.cpp \
    GUI/noshortcutaction.cpp \
    GUI/keyfocustarget.cpp \
    GUI/usagewidget.cpp \
    GUI/verticalwidgetsstack.cpp \
    GUI/valueinput.cpp \
    GUI/twocolumnlayout.cpp \
    GUI/qrealpointvaluedialog.cpp \
    GUI/renderoutputwidget.cpp \
    GUI/qdoubleslider.cpp \
    GUI/qrealanimatorvalueslider.cpp \
    GUI/glwindow.cpp \
    GUI/fontswidget.cpp \
    GUI/filesourcelist.cpp \
    GUI/customfpsdialog.cpp \
    GUI/durationrectsettingsdialog.cpp \
    GUI/fillstrokesettings.cpp \
    GUI/BrushWidgets/arraywidget.cpp \
    GUI/BrushWidgets/brushselectionwidget.cpp \
    GUI/BrushWidgets/flowlayout.cpp \
    Animators/gradientpoints.cpp \
    MovablePoints/gradientpoint.cpp \
    MovablePoints/boxpathpoint.cpp \
    MovablePoints/animatedpoint.cpp \
    MovablePoints/pathpivot.cpp \
    MovablePoints/movablepoint.cpp \
    PropertyUpdaters/transformupdater.cpp \
    PropertyUpdaters/nodepointupdater.cpp \
    PropertyUpdaters/gradientupdater.cpp \
    PropertyUpdaters/strokewidthupdater.cpp \
    PropertyUpdaters/displayedfillstrokesettingsupdater.cpp \
    PropertyUpdaters/pixmapeffectupdater.cpp \
    PropertyUpdaters/animationboxframeupdater.cpp \
    PropertyUpdaters/particlesupdater.cpp \
    PropertyUpdaters/gradientpointsupdater.cpp \
    PropertyUpdaters/singlesoundupdater.cpp \
    PropertyUpdaters/randomqrealgeneratorupdater.cpp \
    PropertyUpdaters/paintboxsizeupdaters.cpp \
    PropertyUpdaters/groupallpathsupdater.cpp \
    PixmapEffects/rastereffects.cpp \
    GUI/ColorWidgets/colorwidgetshaders.cpp \
    taskexecutor.cpp \
    taskscheduler.cpp \
    colorhelpers.cpp \
    Animators/qrealanimatorcreator.cpp \
    Animators/gpueffectanimators.cpp \
    GPUEffects/gpurastereffect.cpp \
    GPUEffects/gpupostprocessor.cpp \
    GPUEffects/gpurastereffectcreator.cpp \
    Paint/fixedtiledsurface.cpp \
    GUI/segment1deditor.cpp \
    GUI/namedcontainer.cpp \
    Animators/brushsettings.cpp \
    Animators/qcubicsegment1danimator.cpp \
    Animators/qrealkey.cpp \
    Paint/autotiledsurface.cpp \
    Paint/brushstroke.cpp \
    Paint/colorconversions.cpp \
    Paint/autotilesdata.cpp \
    FileCacheHandlers/filecachehandler.cpp \
    FileCacheHandlers/imagecachehandler.cpp \
    FileCacheHandlers/imagesequencecachehandler.cpp \
    FileCacheHandlers/animationcachehandler.cpp \
    FileCacheHandlers/videocachehandler.cpp \
    Decode/audiodecode.cpp \
    CacheHandlers/minimalcachecontainer.cpp \
    CacheHandlers/rangecachecontainer.cpp \
    CacheHandlers/imagecachecontainer.cpp \
    CacheHandlers/tmpfilehandlers.cpp \
    CacheHandlers/hddcachablerangecontainer.cpp \
    CacheHandlers/soundcachecontainer.cpp \
    CacheHandlers/soundtmpfilehandlers.cpp \
    CacheHandlers/hddcachablecachehandler.cpp \
    CacheHandlers/minimalcachehandler.cpp \
    CacheHandlers/soundcachehandler.cpp \
    MovablePoints/smartnodepoint.cpp \
    MovablePoints/pathpointshandler.cpp \
    MovablePoints/segment.cpp \
    MovablePoints/smartctrlpoint.cpp \
    Animators/PathAnimators/smartpathcollectionhandler.cpp \
    canvasmouseevents.cpp \
    canvashandlesmartpath.cpp \
    Boxes/smartvectorpath.cpp \
    PropertyUpdaters/smartnodepointupdater.cpp \
    paintsettingsapplier.cpp \
    colorsetting.cpp \
    Animators/gradient.cpp \
    GUI/BrushWidgets/simplebrushwrapper.cpp \
    GUI/BrushWidgets/brushcontexedwrapper.cpp \
    GUI/BrushWidgets/brushwidget.cpp \
    Boxes/waitingforboxload.cpp \
    Boxes/boxesgrouprenderdata.cpp \
    Boxes/canvasrenderdata.cpp \
    Boxes/linkcanvasrenderdata.cpp \
    Boxes/internallinkcanvas.cpp \
    Boxes/internallinkgroupbox.cpp \
    PathEffects/sumpatheffect.cpp \
    pointtypemenu.cpp
HEADERS  += \
    canvas.h \
    GUI/ColorWidgets/colorlabel.h \
    GUI/ColorWidgets/colorsettingswidget.h \
    GUI/ColorWidgets/colorvaluerect.h \
    GUI/ColorWidgets/colorvaluespin.h \
    GUI/ColorWidgets/colorwidget.h \
    GUI/ColorWidgets/h_wheel_sv_triangle.h \
    GUI/ColorWidgets/colorpickingwidget.h \
    GUI/ColorWidgets/savedcolorbutton.h \
    GUI/ColorWidgets/savedcolorswidget.h \
    GUI/ColorWidgets/savedcolorwidgets.h \
    GUI/GradientWidgets/gradientwidget.h \
    GUI/ColorWidgets/glwidget.h \
    svgimporter.h \
    keypoint.h \
    Animators/qstringanimator.h \
    Animators/transformanimator.h \
    Animators/coloranimator.h \
    Animators/qrealanimator.h \
    Animators/qpointfanimator.h \
    Animators/boolanimator.h \
    Animators/intanimator.h \
    Boxes/circle.h \
    Boxes/boundingbox.h \
    Boxes/boxesgroup.h \
    Boxes/imagebox.h \
    Boxes/pathbox.h \
    Boxes/rectangle.h \
    Boxes/textbox.h \
    Animators/effectanimators.h \
    Boxes/animationbox.h \
    pathoperations.h \
    Boxes/linkbox.h \
    Boxes/boxpainthandler.h \
    PixmapEffects/pixmapeffect.h \
    Boxes/particlebox.h \
    GUI/BoxesList/OptimalScrollArea/scrollarea.h \
    GUI/BoxesList/OptimalScrollArea/scrollwidget.h \
    GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h \
    GUI/BoxesList/OptimalScrollArea/singlewidget.h \
    GUI/BoxesList/boxsinglewidget.h \
    GUI/BoxesList/boxscrollwidgetvisiblepart.h \
    GUI/BoxesList/boxscrollwidget.h \
    GUI/BoxesList/boxeslistactionbutton.h \
    clipboardcontainer.h \
    Animators/paintsettings.h \
    Boxes/videobox.h \
    Sound/singlesound.h \
    Sound/soundcomposition.h \
    Boxes/boundingboxrendercontainer.h \
    durationrectangle.h \
    GUI/BoxesList/boolpropertywidget.h \
    Boxes/imagesequencebox.h \
    Boxes/rendercachehandler.h \
    memorychecker.h \
    memoryhandler.h \
    outputgenerator.h \
    GUI/RenderWidgets/renderwidget.h \
    GUI/RenderWidgets/renderinstancewidget.h \
    renderinstancesettings.h \
    GUI/GradientWidgets/gradientslistwidget.h \
    GUI/GradientWidgets/displayedgradientswidget.h \
    GUI/GradientWidgets/currentgradientwidget.h \
    filesourcescache.h \
    global.h \
    GUI/RenderWidgets/closablecontainer.h \
    GUI/RenderWidgets/renderdestinationdialog.h \
    PathEffects/patheffectanimators.h \
    GUI/BoxesList/coloranimatorbutton.h \
    updatable.h \
    GUI/BoxesList/boxtargetwidget.h \
    Properties/boxtargetproperty.h \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.h \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.h \
    Paint/brush.h \
    Paint/surface.h \
    Paint/tile.h \
    Boxes/paintbox.h \
    Paint/animatedsurface.h \
    avfileio.h \
    GUI/paintboxsettingsdialog.h \
    execdelegator.h \
    Paint/tilesdata.h \
    GUI/BoxesList/boxscrollarea.h \
    Properties/comboboxproperty.h \
    Animators/randomqrealgenerator.h \
    Animators/qrealvalueeffect.h \
    Properties/intproperty.h \
    windowsinglewidgettarget.h \
    Boxes/bone.h \
    drawpath.h \
    videoencoder.h \
    GUI/RenderWidgets/rendersettingsdialog.h \
    GUI/RenderWidgets/outputsettingsprofilesdialog.h \
    GUI/RenderWidgets/outputsettingsdisplaywidget.h \
    Boxes/renderdatahandler.h \
    Boxes/boundingboxrenderdata.h \
    application.h \
    PathEffects/displacepatheffect.h \
    PathEffects/lengthpatheffect.h \
    PathEffects/duplicatepatheffect.h \
    PathEffects/solidifypatheffect.h \
    PathEffects/operationpatheffect.h \
    PixmapEffects/blureffect.h \
    PixmapEffects/shadoweffect.h \
    PixmapEffects/lineseffect.h \
    PixmapEffects/circleseffect.h \
    PixmapEffects/swirleffect.h \
    PixmapEffects/oileffect.h \
    PixmapEffects/implodeeffect.h \
    PixmapEffects/desaturateeffect.h \
    PixmapEffects/colorizeeffect.h \
    PixmapEffects/replacecoloreffect.h \
    PixmapEffects/contrasteffect.h \
    PixmapEffects/brightnesseffect.h \
    PixmapEffects/sampledmotionblureffect.h \
    PathEffects/patheffectsinclude.h \
    PixmapEffects/pixmapeffectsinclude.h \
    GUI/actionbutton.h \
    GUI/animationdockwidget.h \
    GUI/animationwidgetscrollbar.h \
    GUI/boxeslistanimationdockwidget.h \
    GUI/boxeslistkeysviewwidget.h \
    GUI/canvaswindow.h \
    GUI/customfpsdialog.h \
    GUI/durationrectsettingsdialog.h \
    GUI/filesourcelist.h \
    GUI/fillstrokesettings.h \
    GUI/fontswidget.h \
    GUI/keyfocustarget.h \
    GUI/keysview.h \
    GUI/mainwindow.h \
    GUI/newcanvasdialog.h \
    GUI/qdoubleslider.h \
    GUI/qrealanimatorvalueslider.h \
    GUI/twocolumnlayout.h \
    GUI/usagewidget.h \
    GUI/valueinput.h \
    GUI/verticalwidgetsstack.h \
    GUI/glwindow.h \
    GUI/noshortcutaction.h \
    GUI/qrealpointvaluedialog.h \
    GUI/renderoutputwidget.h \
    GUI/BrushWidgets/arraywidget.h \
    GUI/BrushWidgets/brushselectionwidget.h \
    GUI/BrushWidgets/flowlayout.h \
    Animators/gradientpoints.h \
    MovablePoints/animatedpoint.h \
    MovablePoints/pathpivot.h \
    MovablePoints/movablepoint.h \
    MovablePoints/gradientpoint.h \
    MovablePoints/boxpathpoint.h \
    PropertyUpdaters/transformupdater.h \
    PropertyUpdaters/nodepointupdater.h \
    PropertyUpdaters/gradientupdater.h \
    PropertyUpdaters/strokewidthupdater.h \
    PropertyUpdaters/displayedfillstrokesettingsupdater.h \
    PropertyUpdaters/pixmapeffectupdater.h \
    PropertyUpdaters/animationboxframeupdater.h \
    PropertyUpdaters/particlesupdater.h \
    PropertyUpdaters/gradientpointsupdater.h \
    PropertyUpdaters/singlesoundupdater.h \
    PropertyUpdaters/randomqrealgeneratorupdater.h \
    PropertyUpdaters/paintboxsizeupdaters.h \
    PropertyUpdaters/groupallpathsupdater.h \
    PixmapEffects/rastereffects.h \
    GUI/ColorWidgets/colorwidgetshaders.h \
    taskexecutor.h \
    taskscheduler.h \
    colorhelpers.h \
    Animators/qrealanimatorcreator.h \
    Animators/intanimatorcreator.h \
    Animators/gpueffectanimators.h \
    GPUEffects/gpurastereffect.h \
    GPUEffects/gpupostprocessor.h \
    GPUEffects/gpurastereffectcreator.h \
    Paint/fixedtiledsurface.h \
    GUI/segment1deditor.h \
    GUI/namedcontainer.h \
    Animators/brushsettings.h \
    Animators/qcubicsegment1danimator.h \
    Animators/qrealkey.h \
    Paint/autotiledsurface.h \
    Paint/brushstroke.h \
    Paint/colorconversions.h \
    Paint/autotilesdata.h \
    FileCacheHandlers/filecachehandler.h \
    FileCacheHandlers/imagecachehandler.h \
    FileCacheHandlers/imagesequencecachehandler.h \
    FileCacheHandlers/animationcachehandler.h \
    FileCacheHandlers/videocachehandler.h \
    Decode/audiodecode.h \
    CacheHandlers/minimalcachecontainer.h \
    CacheHandlers/rangecachecontainer.h \
    CacheHandlers/imagecachecontainer.h \
    CacheHandlers/tmpfilehandlers.h \
    CacheHandlers/hddcachablerangecontainer.h \
    CacheHandlers/soundcachecontainer.h \
    CacheHandlers/soundtmpfilehandlers.h \
    CacheHandlers/hddcachablecachehandler.h \
    CacheHandlers/minimalcachehandler.h \
    CacheHandlers/soundcachehandler.h \
    MovablePoints/smartnodepoint.h \
    MovablePoints/pathpointshandler.h \
    MovablePoints/segment.h \
    MovablePoints/smartctrlpoint.h \
    Animators/PathAnimators/smartpathcollectionhandler.h \
    Boxes/smartvectorpath.h \
    PropertyUpdaters/smartnodepointupdater.h \
    paintsettingsapplier.h \
    colorsetting.h \
    Animators/gradient.h \
    GUI/BrushWidgets/simplebrushwrapper.h \
    GUI/BrushWidgets/brushcontexedwrapper.h \
    GUI/BrushWidgets/brushwidget.h \
    Boxes/waitingforboxload.h \
    Boxes/boxesgrouprenderdata.h \
    Boxes/canvasrenderdata.h \
    Boxes/linkcanvasrenderdata.h \
    Boxes/internallinkcanvas.h \
    Boxes/internallinkgroupbox.h \
    PathEffects/sumpatheffect.h \
    pointtypemenu.h

RESOURCES += \
    resources.qrc

DISTFILES += \
    stylesheet.qss \
    pixmaps/crosshair_closed.png \
    pixmaps/cursor_add.png \
    pixmaps/cursor_arrow.png \
    pixmaps/cursor_arrow_forbidden.png \
    pixmaps/cursor_crosshair_open.png \
    pixmaps/cursor_crosshair_precise_open.png \
    pixmaps/cursor_forbidden_everywhere.png \
    pixmaps/cursor_hand_closed.png \
    pixmaps/cursor_hand_open.png \
    pixmaps/cursor_move_n_s.png \
    pixmaps/cursor_move_ne_sw.png \
    pixmaps/cursor_move_nw_se.png \
    pixmaps/cursor_move_w_e.png \
    pixmaps/cursor_pencil.png \
    pixmaps/cursor_remove.png \
    pixmaps/layer_duplicate.png \
    pixmaps/layers.png \
    pixmaps/plus.png \
    project/base_screen.png \
    pixmaps/icons/tango_icons.svg \
    project/base_screen.svg \
    pixmaps/cursor-3dbox.xpm \
    pixmaps/cursor-adj-a.xpm \
    pixmaps/cursor-adj-h.xpm \
    pixmaps/cursor-adj-l.xpm \
    pixmaps/cursor-adj-s.xpm \
    pixmaps/cursor-arc.xpm \
    pixmaps/cursor-arrow.xpm \
    pixmaps/cursor-attract.xpm \
    pixmaps/cursor-calligraphy.xpm \
    pixmaps/cursor-color.xpm \
    pixmaps/cursor-connector.xpm \
    pixmaps/cursor-crosshairs.xpm \
    pixmaps/cursor-dropper-f.xpm \
    pixmaps/cursor-dropper-s.xpm \
    pixmaps/cursor-eraser.xpm \
    pixmaps/cursor-gradient-add.xpm \
    pixmaps/cursor-gradient.xpm \
    pixmaps/cursor-measure.xpm \
    pixmaps/cursor-node-d.xpm \
    pixmaps/cursor-node-m.xpm \
    pixmaps/cursor-paintbucket.xpm \
    pixmaps/cursor-pencil.xpm \
    pixmaps/cursor-push.xpm \
    pixmaps/cursor-repel.xpm \
    pixmaps/cursor-roughen.xpm \
    pixmaps/cursor-select-d.xpm \
    pixmaps/cursor-select-m.xpm \
    pixmaps/cursor-spiral.xpm \
    pixmaps/cursor-spray-move.xpm \
    pixmaps/cursor-spray.xpm \
    pixmaps/cursor-star.xpm \
    pixmaps/cursor-text-insert.xpm \
    pixmaps/cursor-thicken.xpm \
    pixmaps/cursor-thin.xpm \
    pixmaps/cursor-tweak-less.xpm \
    pixmaps/cursor-tweak-more.xpm \
    pixmaps/cursor-tweak-move-in.xpm \
    pixmaps/cursor-tweak-move-jitter.xpm \
    pixmaps/cursor-tweak-move-out.xpm \
    pixmaps/cursor-tweak-move.xpm \
    pixmaps/cursor-tweak-rotate-clockwise.xpm \
    pixmaps/cursor-tweak-rotate-counterclockwise.xpm \
    pixmaps/cursor-tweak-scale-down.xpm \
    pixmaps/cursor-tweak-scale-up.xpm \
    pixmaps/cursor-zoom-out.xpm \
    pixmaps/cursor-zoom.xpm \
    pixmaps/handles.xpm
