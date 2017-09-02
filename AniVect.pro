#-------------------------------------------------
#
# Project created by QtCreator 2016-08-06T14:25:20
#
#-------------------------------------------------

QT += multimedia core gui svg opengl sql xml #widgets-private # gui-private core-private
LIBS += -lavutil -lavformat -lavcodec -lswscale -lswresample -ltcmalloc

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
        mainwindow.cpp \
    canvas.cpp \
    undoredo.cpp \
    connectedtomainwindow.cpp \
    movablepoint.cpp \
    pathpivot.cpp \
    canvasmouseinteractions.cpp \
    ctrlpoint.cpp \
    Colors/ColorWidgets/colorlabel.cpp \
    Colors/ColorWidgets/colorsettingswidget.cpp \
    Colors/ColorWidgets/colorvaluerect.cpp \
    Colors/ColorWidgets/colorvaluespin.cpp \
    Colors/ColorWidgets/colorwidget.cpp \
    Colors/ColorWidgets/h_wheel_sv_triangle.cpp \
    Colors/color.cpp \
    Colors/colorpickingwidget.cpp \
    Colors/savedcolorbutton.cpp \
    Colors/savedcolorswidget.cpp \
    Colors/savedcolorwidgets.cpp \
    Colors/helpers.cpp \
    fillstrokesettings.cpp \
    Colors/ColorWidgets/gradientwidget.cpp \
    Colors/ColorWidgets/glwidget.cpp \
    gradientpoint.cpp \
    svgimporter.cpp \
    transformable.cpp \
    animationdockwidget.cpp \
    qrealpointvaluedialog.cpp \
    boxeslistanimationdockwidget.cpp \
    qrealkey.cpp \
    qrealpoint.cpp \
    pointhelpers.cpp \
    updatescheduler.cpp \
    graphboxeslist.cpp \
    keysview.cpp \
    gradientpoints.cpp \
    fontswidget.cpp \
    paintcontroler.cpp \
    qdoubleslider.cpp \
    qrealanimatorvalueslider.cpp \
    renderoutputwidget.cpp \
    Animators/transformanimator.cpp \
    Animators/animator.cpp \
    Animators/boolanimator.cpp \
    Animators/coloranimator.cpp \
    Animators/complexanimator.cpp \
    Animators/pathanimator.cpp \
    Animators/qstringanimator.cpp \
    Animators/qpointfanimator.cpp \
    Animators/animatorupdater.cpp \
    Boxes/circle.cpp \
    Boxes/textbox.cpp \
    Boxes/vectorpath.cpp \
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
    actionbutton.cpp \
    Boxes/boxpainthandler.cpp \
    newcanvasdialog.cpp \
    PixmapEffects/brusheffect.cpp \
    PixmapEffects/fmt_filters.cpp \
    PixmapEffects/pixmapeffect.cpp \
    Boxes/particlebox.cpp \
    BoxesList/OptimalScrollArea/scrollarea.cpp \
    BoxesList/OptimalScrollArea/scrollwidget.cpp \
    BoxesList/OptimalScrollArea/singlewidgettarget.cpp \
    BoxesList/OptimalScrollArea/singlewidgetabstraction.cpp \
    BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.cpp \
    BoxesList/OptimalScrollArea/singlewidget.cpp \
    BoxesList/boxsinglewidget.cpp \
    BoxesList/boxscrollwidgetvisiblepart.cpp \
    BoxesList/boxscrollwidget.cpp \
    BoxesList/boxeslistactionbutton.cpp \
    canvasselectedpointsactions.cpp \
    canvasselectedboxesactions.cpp \
    clipboardcontainer.cpp \
    Animators/paintsettings.cpp \
    edge.cpp \
    Boxes/videobox.cpp \
    Sound/singlesound.cpp \
    Sound/soundcomposition.cpp \
    Boxes/boundingboxrendercontainer.cpp \
    Properties/property.cpp \
    boxeslistkeysviewwidget.cpp \
    animationwidgetscrollbar.cpp \
    verticalwidgetsstack.cpp \
    durationrectangle.cpp \
    Properties/boolproperty.cpp \
    BoxesList/boolpropertywidget.cpp \
    Boxes/imagesequencebox.cpp \
    key.cpp \
    Boxes/rendercachehandler.cpp \
    memorychecker.cpp \
    memoryhandler.cpp \
    boxpathpoint.cpp \
    Bones/bonepoint.cpp \
    outputgenerator.cpp \
    RenderWidget/renderwidget.cpp \
    RenderWidget/renderinstancewidget.cpp \
    RenderWidget/renderinstancesettings.cpp \
    Gradients/gradientslistwidget.cpp \
    Gradients/displayedgradientswidget.cpp \
    Gradients/currentgradientwidget.cpp \
    filesourcescache.cpp \
    glwindow.cpp \
    canvaswindow.cpp \
    skqtconversions.cpp \
    AddInclude/SkStroke.cpp \
    AddInclude/SkGeometry.cpp \
    AddInclude/SkStrokerPriv.cpp \
    canvaswidget.cpp \
    RenderWidget/closablecontainer.cpp \
    RenderWidget/renderdestinationdialog.cpp \
    sqltablecreation.cpp \
    PathEffects/patheffect.cpp \
    PathEffects/patheffectanimators.cpp \
    BoxesList/coloranimatorbutton.cpp \
    keyfocustarget.cpp \
    noshortcutaction.cpp \
    updatable.cpp \
    Animators/PathAnimators/singlepathanimator.cpp \
    Animators/PathAnimators/singlevectorpathanimator.cpp \
    BoxesList/boxtargetwidget.cpp \
    Properties/boxtargetproperty.cpp \
    BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.cpp \
    BoxesList/OptimalScrollArea/minimalscrollwidget.cpp \
    filesourcelist.cpp \
    Paint/BrushSettings/brushsettingswidget.cpp \
    Paint/BrushSettings/brushsettingwidget.cpp \
    Paint/PaintLib/brush.cpp \
    Paint/PaintLib/surface.cpp \
    Paint/PaintLib/tile.cpp \
    Boxes/paintbox.cpp \
    Paint/PaintLib/animatedsurface.cpp \
    Animators/PathAnimators/pathkey.cpp \
    Animators/PathAnimators/vectorpathanimator.cpp \
    pointanimator.cpp \
    nodepoint.cpp \
    avfileio.cpp

HEADERS  += mainwindow.h \
    canvas.h \
    undoredo.h \
    updatescheduler.h \
    connectedtomainwindow.h \
    movablepoint.h \
    pathpivot.h \
    ctrlpoint.h \
    Colors/ColorWidgets/colorlabel.h \
    Colors/ColorWidgets/colorsettingswidget.h \
    Colors/ColorWidgets/colorvaluerect.h \
    Colors/ColorWidgets/colorvaluespin.h \
    Colors/ColorWidgets/colorwidget.h \
    Colors/ColorWidgets/h_wheel_sv_triangle.h \
    Colors/color.h \
    Colors/colorpickingwidget.h \
    Colors/savedcolorbutton.h \
    Colors/savedcolorswidget.h \
    Colors/savedcolorwidgets.h \
    Colors/helpers.h \
    fillstrokesettings.h \
    Colors/ColorWidgets/gradientwidget.h \
    Colors/ColorWidgets/glwidget.h \
    gradientpoint.h \
    svgimporter.h \
    transformable.h \
    animationdockwidget.h \
    qrealpointvaluedialog.h \
    boxeslistanimationdockwidget.h \
    qrealkey.h \
    qrealpoint.h \
    pointhelpers.h \
    keysview.h \
    gradientpoints.h \
    fontswidget.h \
    paintcontroler.h \
    qdoubleslider.h \
    qrealanimatorvalueslider.h \
    renderoutputwidget.h \
    keypoint.h \
    Animators/qstringanimator.h \
    Animators/transformanimator.h \
    Animators/complexanimator.h \
    Animators/coloranimator.h \
    Animators/animator.h \
    Animators/qrealanimator.h \
    Animators/qpointfanimator.h \
    Animators/boolanimator.h \
    Animators/animatorupdater.h \
    Animators/intanimator.h \
    Boxes/circle.h \
    Boxes/boundingbox.h \
    Boxes/boxesgroup.h \
    Boxes/imagebox.h \
    Boxes/pathbox.h \
    Animators/pathanimator.h \
    Boxes/rectangle.h \
    Boxes/textbox.h \
    Boxes/vectorpath.h \
    Animators/effectanimators.h \
    Boxes/animationbox.h \
    pathoperations.h \
    Boxes/linkbox.h \
    actionbutton.h \
    Boxes/boxpainthandler.h \
    newcanvasdialog.h \
    PixmapEffects/brusheffect.h \
    PixmapEffects/pixmapeffect.h \
    PixmapEffects/fmt_filters.h \
    Boxes/particlebox.h \
    BoxesList/OptimalScrollArea/scrollarea.h \
    BoxesList/OptimalScrollArea/scrollwidget.h \
    BoxesList/OptimalScrollArea/singlewidgettarget.h \
    BoxesList/OptimalScrollArea/singlewidgetabstraction.h \
    BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h \
    BoxesList/OptimalScrollArea/singlewidget.h \
    BoxesList/boxsinglewidget.h \
    BoxesList/boxscrollwidgetvisiblepart.h \
    BoxesList/boxscrollwidget.h \
    BoxesList/boxeslistactionbutton.h \
    clipboardcontainer.h \
    Animators/paintsettings.h \
    edge.h \
    Boxes/videobox.h \
    Sound/singlesound.h \
    Sound/soundcomposition.h \
    Boxes/boundingboxrendercontainer.h \
    Properties/property.h \
    boxeslistkeysviewwidget.h \
    animationwidgetscrollbar.h \
    verticalwidgetsstack.h \
    durationrectangle.h \
    Properties/boolproperty.h \
    BoxesList/boolpropertywidget.h \
    Boxes/imagesequencebox.h \
    key.h \
    Boxes/rendercachehandler.h \
    memorychecker.h \
    memoryhandler.h \
    selfref.h \
    boxpathpoint.h \
    Bones/bonepoint.h \
    outputgenerator.h \
    RenderWidget/renderwidget.h \
    RenderWidget/renderinstancewidget.h \
    RenderWidget/renderinstancesettings.h \
    Gradients/gradientslistwidget.h \
    Gradients/displayedgradientswidget.h \
    Gradients/currentgradientwidget.h \
    filesourcescache.h \
    global.h \
    glwindow.h \
    canvaswindow.h \
    skqtconversions.h \
    AddInclude/SkStroke.h \
    AddInclude/SkPaintDefaults.h \
    AddInclude/SkGeometry.h \
    AddInclude/SkStrokerPriv.h \
    AddInclude/SkNx.h \
    canvaswidget.h \
    skiaincludes.h \
    AddInclude/SkPathPriv.h \
    skiadefines.h \
    RenderWidget/closablecontainer.h \
    RenderWidget/renderdestinationdialog.h \
    PathEffects/patheffect.h \
    PathEffects/patheffectanimators.h \
    BoxesList/coloranimatorbutton.h \
    keyfocustarget.h \
    noshortcutaction.h \
    updatable.h \
    Animators/PathAnimators/singlepathanimator.h \
    Animators/PathAnimators/singlevectorpathanimator.h \
    BoxesList/boxtargetwidget.h \
    Properties/boxtargetproperty.h \
    BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.h \
    BoxesList/OptimalScrollArea/minimalscrollwidget.h \
    filesourcelist.h \
    Paint/BrushSettings/brushsettingswidget.h \
    Paint/BrushSettings/brushsettingwidget.h \
    Paint/PaintLib/brush.h \
    Paint/PaintLib/surface.h \
    Paint/PaintLib/tile.h \
    Boxes/paintbox.h \
    Paint/PaintLib/animatedsurface.h \
    Animators/PathAnimators/pathkey.h \
    Animators/PathAnimators/vectorpathanimator.h \
    pointanimator.h \
    nodepoint.h \
    avfileio.h

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
