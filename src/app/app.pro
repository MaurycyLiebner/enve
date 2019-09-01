# enve - 2D animations software
# Copyright (C) 2016-2019 Maurycy Liebner

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
# Project created by QtCreator 2016-08-06T14:25:20
#
#-------------------------------------------------

QT += multimedia core gui svg opengl sql qml xml concurrent
LIBS += -lavutil -lavformat -lavcodec -lswscale -lswresample -lavresample -ltcmalloc

ENVE_FOLDER = $$PWD/../..
SKIA_FOLDER = $$ENVE_FOLDER/third_party/skia
LIBMYPAINT_FOLDER = $$ENVE_FOLDER/third_party/libmypaint-1.3.0

INCLUDEPATH += ../core
DEPENDPATH += ../core

LIBS += -L$$OUT_PWD/../core -lenvecore

INCLUDEPATH += $$LIBMYPAINT_FOLDER/include
LIBS += -L$$LIBMYPAINT_FOLDER/.libs -lmypaint -lgobject-2.0 -lglib-2.0 -ljson-c

INCLUDEPATH += $$SKIA_FOLDER

CONFIG(debug, debug|release) {
    LIBS += -L$$SKIA_FOLDER/out/Debug
} else {
    LIBS += -L$$SKIA_FOLDER/out/Release
    QMAKE_CFLAGS -= -O2
    QMAKE_CFLAGS -= -O1
    QMAKE_CXXFLAGS -= -O2
    QMAKE_CXXFLAGS -= -O1
    QMAKE_CFLAGS = -m64 -O3
    QMAKE_LFLAGS = -m64 -O3
    QMAKE_CXXFLAGS = -m64 -O3
}

QMAKE_CXXFLAGS += -fopenmp
LIBS += -lskia -lpthread -lfreetype -lpng -ldl -fopenmp# -lX11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = enve
TEMPLATE = app

SOURCES += main.cpp\
    GUI/audiohandler.cpp \
    GUI/canvasbasewrappernode.cpp \
    GUI/canvaswindowevents.cpp \
    GUI/canvaswrappernode.cpp \
    GUI/changewidthwidget.cpp \
    GUI/ekeyfilter.cpp \
    GUI/envelicense.cpp \
    GUI/envesplash.cpp \
    GUI/ewidgetsimpl.cpp \
    GUI/layouthandler.cpp \
    GUI/scenechooser.cpp \
    GUI/timelinebasewrappernode.cpp \
    GUI/timelinedockwidget.cpp \
    GUI/timelinewidget.cpp \
    GUI/timelinewrappernode.cpp \
    GUI/welcomedialog.cpp \
    GUI/widgetstack.cpp \
    GUI/ColorWidgets/colorlabel.cpp \
    GUI/ColorWidgets/colorsettingswidget.cpp \
    GUI/ColorWidgets/colorvaluerect.cpp \
    GUI/ColorWidgets/colorwidget.cpp \
    GUI/ColorWidgets/h_wheel_sv_triangle.cpp \
    GUI/ColorWidgets/colorpickingwidget.cpp \
    GUI/ColorWidgets/savedcolorbutton.cpp \
    GUI/ColorWidgets/savedcolorswidget.cpp \
    GUI/ColorWidgets/savedcolorwidgets.cpp \
    GUI/GradientWidgets/gradientwidget.cpp \
    GUI/ColorWidgets/glwidget.cpp \
    GUI/widgetwrappernode.cpp \
    GUI/wrappernode.cpp \
    effectsloader.cpp \
    esettings.cpp \
    evfileio.cpp \
    hardwareinfo.cpp \
    iconloader.cpp \
    outputsettings.cpp \
    renderhandler.cpp \
    svgimporter.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollarea.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollwidget.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.cpp \
    GUI/BoxesList/OptimalScrollArea/singlewidget.cpp \
    GUI/BoxesList/boxsinglewidget.cpp \
    GUI/BoxesList/boxscrollwidgetvisiblepart.cpp \
    GUI/BoxesList/boxscrollwidget.cpp \
    GUI/BoxesList/boolpropertywidget.cpp \
    memorychecker.cpp \
    memoryhandler.cpp \
    GUI/RenderWidgets/renderwidget.cpp \
    GUI/RenderWidgets/renderinstancewidget.cpp \
    renderinstancesettings.cpp \
    GUI/GradientWidgets/gradientslistwidget.cpp \
    GUI/GradientWidgets/displayedgradientswidget.cpp \
    GUI/GradientWidgets/currentgradientwidget.cpp \
    GUI/RenderWidgets/closablecontainer.cpp \
    GUI/RenderWidgets/renderdestinationdialog.cpp \
    GUI/BoxesList/boxtargetwidget.cpp \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.cpp \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.cpp \
    GUI/paintboxsettingsdialog.cpp \
    execdelegator.cpp \
    GUI/BoxesList/boxscrollarea.cpp \
    videoencoder.cpp \
    GUI/RenderWidgets/rendersettingsdialog.cpp \
    GUI/RenderWidgets/outputsettingsprofilesdialog.cpp \
    GUI/RenderWidgets/outputsettingsdisplaywidget.cpp \
    GUI/actionbutton.cpp \
    GUI/animationdockwidget.cpp \
    GUI/animationwidgetscrollbar.cpp \
    GUI/canvaswindow.cpp \
    GUI/graphboxeslist.cpp \
    GUI/keysview.cpp \
    GUI/mainwindow.cpp \
    GUI/noshortcutaction.cpp \
    GUI/keyfocustarget.cpp \
    GUI/usagewidget.cpp \
    GUI/qrealpointvaluedialog.cpp \
    GUI/renderoutputwidget.cpp \
    GUI/qdoubleslider.cpp \
    GUI/qrealanimatorvalueslider.cpp \
    GUI/glwindow.cpp \
    GUI/fontswidget.cpp \
    GUI/filesourcelist.cpp \
    GUI/customfpsdialog.cpp \
    GUI/fillstrokesettings.cpp \
    GUI/BrushWidgets/arraywidget.cpp \
    GUI/BrushWidgets/brushselectionwidget.cpp \
    GUI/BrushWidgets/flowlayout.cpp \
    GUI/ColorWidgets/colorwidgetshaders.cpp \
    GUI/segment1deditor.cpp \
    GUI/namedcontainer.cpp \
    Animators/qcubicsegment1danimator.cpp \
    GUI/BrushWidgets/brushcontexedwrapper.cpp \
    GUI/BrushWidgets/brushwidget.cpp

HEADERS  += \
    GUI/audiohandler.h \
    GUI/canvasbasewrappernode.h \
    GUI/canvaswrappernode.h \
    GUI/changewidthwidget.h \
    GUI/ekeyfilter.h \
    GUI/envelicense.h \
    GUI/envesplash.h \
    GUI/ewidgetsimpl.h \
    GUI/layouthandler.h \
    GUI/scenechooser.h \
    GUI/timelinebasewrappernode.h \
    GUI/timelinedockwidget.h \
    GUI/timelinewidget.h \
    GUI/timelinewrappernode.h \
    GUI/welcomedialog.h \
    GUI/widgetstack.h \
    GUI/ColorWidgets/colorlabel.h \
    GUI/ColorWidgets/colorsettingswidget.h \
    GUI/ColorWidgets/colorvaluerect.h \
    GUI/ColorWidgets/colorwidget.h \
    GUI/ColorWidgets/h_wheel_sv_triangle.h \
    GUI/ColorWidgets/colorpickingwidget.h \
    GUI/ColorWidgets/savedcolorbutton.h \
    GUI/ColorWidgets/savedcolorswidget.h \
    GUI/ColorWidgets/savedcolorwidgets.h \
    GUI/GradientWidgets/gradientwidget.h \
    GUI/ColorWidgets/glwidget.h \
    GUI/widgetwrappernode.h \
    GUI/wrappernode.h \
    effectsloader.h \
    esettings.h \
    hardwareinfo.h \
    iconloader.h \
    outputsettings.h \
    renderhandler.h \
    svgimporter.h \
    keypoint.h \
    GUI/BoxesList/OptimalScrollArea/scrollarea.h \
    GUI/BoxesList/OptimalScrollArea/scrollwidget.h \
    GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h \
    GUI/BoxesList/OptimalScrollArea/singlewidget.h \
    GUI/BoxesList/boxsinglewidget.h \
    GUI/BoxesList/boxscrollwidgetvisiblepart.h \
    GUI/BoxesList/boxscrollwidget.h \
    GUI/BoxesList/boolpropertywidget.h \
    memorychecker.h \
    memoryhandler.h \
    GUI/RenderWidgets/renderwidget.h \
    GUI/RenderWidgets/renderinstancewidget.h \
    renderinstancesettings.h \
    GUI/GradientWidgets/gradientslistwidget.h \
    GUI/GradientWidgets/displayedgradientswidget.h \
    GUI/GradientWidgets/currentgradientwidget.h \
    GUI/RenderWidgets/closablecontainer.h \
    GUI/RenderWidgets/renderdestinationdialog.h \
    GUI/BoxesList/boxtargetwidget.h \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.h \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.h \
    GUI/paintboxsettingsdialog.h \
    execdelegator.h \
    GUI/BoxesList/boxscrollarea.h \
    videoencoder.h \
    GUI/RenderWidgets/rendersettingsdialog.h \
    GUI/RenderWidgets/outputsettingsprofilesdialog.h \
    GUI/RenderWidgets/outputsettingsdisplaywidget.h \
    GUI/actionbutton.h \
    GUI/animationdockwidget.h \
    GUI/animationwidgetscrollbar.h \
    GUI/canvaswindow.h \
    GUI/customfpsdialog.h \
    GUI/filesourcelist.h \
    GUI/fillstrokesettings.h \
    GUI/fontswidget.h \
    GUI/keyfocustarget.h \
    GUI/keysview.h \
    GUI/mainwindow.h \
    GUI/qdoubleslider.h \
    GUI/qrealanimatorvalueslider.h \
    GUI/usagewidget.h \
    GUI/glwindow.h \
    GUI/noshortcutaction.h \
    GUI/qrealpointvaluedialog.h \
    GUI/renderoutputwidget.h \
    GUI/BrushWidgets/arraywidget.h \
    GUI/BrushWidgets/brushselectionwidget.h \
    GUI/BrushWidgets/flowlayout.h \
    GUI/ColorWidgets/colorwidgetshaders.h \
    GUI/segment1deditor.h \
    GUI/namedcontainer.h \
    Animators/qcubicsegment1danimator.h \
    GUI/BrushWidgets/brushcontexedwrapper.h \
    GUI/BrushWidgets/brushwidget.h \
    boxtypemenu.h

RESOURCES += resources.qrc

toolbarButtonsBg.files = $$files(icons/toolbarButtons/*)
RESOURCES += toolbarButtonsBg

toolbarButtonsPlain.files = $$files(icons/toolbarButtons/plain/*)
RESOURCES += toolbarButtonsPlain

toolbarButtonsCheckable.files = $$files(icons/toolbarButtons/checkable/*)
RESOURCES += toolbarButtonsCheckable

noInterpolation.files = $$files(icons/noInterpolation/*)
RESOURCES += noInterpolation

plain.files = $$files(icons/*)
RESOURCES += plain
