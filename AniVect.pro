#-------------------------------------------------
#
# Project created by QtCreator 2016-08-06T14:25:20
#
#-------------------------------------------------

QT       += multimedia core gui svg opengl sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AniVect
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    pathpoint.cpp \
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
    animatonwidgetscrollbar.cpp \
    animationdockwidget.cpp \
    qrealpointvaluedialog.cpp \
    boxeslistanimationdockwidget.cpp \
    qrealkey.cpp \
    qrealpoint.cpp \
    pointhelpers.cpp \
    updatescheduler.cpp \
    smartpointertarget.cpp \
    graphboxeslist.cpp \
    keysview.cpp \
    gradientpoints.cpp \
    fontswidget.cpp \
    paintcontroler.cpp \
    fmt_filters.cpp \
    vectorshapesmenu.cpp \
    qdoubleslider.cpp \
    qrealanimatorvalueslider.cpp \
    vectorshapeswidget.cpp \
    renderoutputwidget.cpp \
    canvaswidget.cpp \
    BoxesList/boxeslistwidget.cpp \
    BoxesList/widgetcontainer.cpp \
    BoxesList/qrealanimatoritemwidget.cpp \
    BoxesList/complexanimatoritemwidget.cpp \
    BoxesList/complexanimatoritemwidgetcontainer.cpp \
    BoxesList/boxitemwidgetcontainer.cpp \
    BoxesList/boxitemwidget.cpp \
    BoxesList/boxesgroupwidgetcontainer.cpp \
    Sound/singlesound.cpp \
    Sound/soundcomposition.cpp \
    ObjectSettings/objectsettingswidget.cpp \
    Animators/animatorscollection.cpp \
    Animators/transformanimator.cpp \
    Animators/animator.cpp \
    Animators/boolanimator.cpp \
    Animators/coloranimator.cpp \
    Animators/complexanimator.cpp \
    Animators/pathanimator.cpp \
    Animators/qstringanimator.cpp \
    pixmapeffect.cpp \
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
    BoxesList/effectanimatorsitemwidgetcontainer.cpp \
    Boxes/animationbox.cpp \
    pathoperations.cpp

HEADERS  += mainwindow.h \
    pathpoint.h \
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
    animatonwidgetscrollbar.h \
    animationdockwidget.h \
    qrealpointvaluedialog.h \
    boxeslistanimationdockwidget.h \
    qrealkey.h \
    qrealpoint.h \
    pointhelpers.h \
    smartpointertarget.h \
    keysview.h \
    gradientpoints.h \
    fontswidget.h \
    paintcontroler.h \
    fmt_filters.h \
    pixmapeffect.h \
    vectorshapesmenu.h \
    qdoubleslider.h \
    qrealanimatorvalueslider.h \
    vectorshapeswidget.h \
    renderoutputwidget.h \
    canvaswidget.h \
    keypoint.h \
    BoxesList/boxeslistwidget.h \
    BoxesList/widgetcontainer.h \
    BoxesList/qrealanimatoritemwidget.h \
    BoxesList/complexanimatoritemwidget.h \
    BoxesList/complexanimatoritemwidgetcontainer.h \
    BoxesList/boxitemwidgetcontainer.h \
    BoxesList/boxitemwidget.h \
    BoxesList/boxesgroupwidgetcontainer.h \
    Sound/singlesound.h \
    Sound/soundcomposition.h \
    ObjectSettings/objectsettingswidget.h \
    Animators/qstringanimator.h \
    Animators/transformanimator.h \
    Animators/complexanimator.h \
    Animators/coloranimator.h \
    Animators/animator.h \
    Animators/qrealanimator.h \
    Animators/qpointfanimator.h \
    Animators/animatorscollection.h \
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
    BoxesList/effectanimatorsitemwidgetcontainer.h \
    Boxes/animationbox.h \
    pathoperations.h
