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
# Project created by QtCreator 2016-08-06T14:25:20
#
#-------------------------------------------------

VERSION = 0.0.0

QT += multimedia core gui svg opengl sql qml xml concurrent
LIBS += -lavutil -lavformat -lavcodec -lswscale -lswresample -lavresample
CONFIG += c++14

ENVE_FOLDER = $$PWD/../..
THIRD_PARTY_FOLDER =  $$ENVE_FOLDER/third_party
SKIA_FOLDER = $$THIRD_PARTY_FOLDER/skia
LIBMYPAINT_FOLDER = $$THIRD_PARTY_FOLDER/libmypaint-1.5.0
QUAZIP_FOLDER = $$THIRD_PARTY_FOLDER/quazip-0.8.1
GPERFTOOLS_FOLDER = $$THIRD_PARTY_FOLDER/gperftools-2.7-enve-mod
QSCINTILLA_FOLDER = $$THIRD_PARTY_FOLDER/QScintilla-2.11.4/Qt4Qt5

INCLUDEPATH += ../core
DEPENDPATH += ../core

LIBS += -L$$OUT_PWD/../core -lenvecore

INCLUDEPATH += $$LIBMYPAINT_FOLDER/include
LIBS += -L$$LIBMYPAINT_FOLDER/.libs -lmypaint -lgobject-2.0 -lglib-2.0 -ljson-c

INCLUDEPATH += $$QUAZIP_FOLDER
LIBS += -L$$QUAZIP_FOLDER/quazip -lquazip

INCLUDEPATH += $$GPERFTOOLS_FOLDER/include
LIBS += -L$$GPERFTOOLS_FOLDER/.libs -ltcmalloc

INCLUDEPATH += $$QSCINTILLA_FOLDER
LIBS += -L$$QSCINTILLA_FOLDER -lqscintilla2_qt5

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
    GUI/BoxesList/OptimalScrollArea/scrollvisiblepartbase.cpp \
    GUI/BoxesList/boxscroller.cpp \
    GUI/BrushWidgets/bookmarkedbrushes.cpp \
    GUI/BrushWidgets/brushlabel.cpp \
    GUI/ColorWidgets/bookmarkedcolors.cpp \
    GUI/Dialogs/dialogsinterfaceimpl.cpp \
    GUI/Dialogs/durationrectsettingsdialog.cpp \
    GUI/Expressions/expressiondialog.cpp \
    GUI/Expressions/expressioneditor.cpp \
    GUI/Expressions/expressionhighlighter.cpp \
    GUI/RenderWidgets/outputsettingsdialog.cpp \
    GUI/RenderWidgets/rendersettingsdialog.cpp \
    GUI/RenderWidgets/rendersettingsdisplaywidget.cpp \
    GUI/Settings/externalappssettingswidget.cpp \
    GUI/Settings/performancesettingswidget.cpp \
    GUI/Settings/settingsdialog.cpp \
    GUI/Settings/settingswidget.cpp \
    GUI/Settings/timelinesettingswidget.cpp \
    GUI/audiohandler.cpp \
    GUI/bookmarkedwidget.cpp \
    GUI/buttonbase.cpp \
    GUI/buttonslist.cpp \
    GUI/canvasbasewrappernode.cpp \
    GUI/canvaswindowevents.cpp \
    GUI/canvaswrappernode.cpp \
    GUI/centralwidget.cpp \
    GUI/changewidthwidget.cpp \
    GUI/closesignalingdockwidget.cpp \
    GUI/ecombobox.cpp \
    GUI/ekeyfilter.cpp \
    GUI/envelicense.cpp \
    GUI/envesplash.cpp \
    GUI/ewidgetsimpl.cpp \
    GUI/layouthandler.cpp \
    GUI/scenechooser.cpp \
    GUI/switchbutton.cpp \
    GUI/timelinebasewrappernode.cpp \
    GUI/timelinedockwidget.cpp \
    GUI/timelinehighlightwidget.cpp \
    GUI/timelinewidget.cpp \
    GUI/timelinewrappernode.cpp \
    GUI/triggerlabel.cpp \
    GUI/twocolumnlayout.cpp \
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
    GUI/GradientWidgets/gradientwidget.cpp \
    GUI/ColorWidgets/glwidget.cpp \
    GUI/widgetwrappernode.cpp \
    GUI/wrappernode.cpp \
    effectsloader.cpp \
    eimporters.cpp \
    evfileio.cpp \
    hardwareinfo.cpp \
    iconloader.cpp \
    outputsettings.cpp \
    renderhandler.cpp \
    rendersettings.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollarea.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollwidget.cpp \
    GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.cpp \
    GUI/BoxesList/OptimalScrollArea/singlewidget.cpp \
    GUI/BoxesList/boxsinglewidget.cpp \
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
    GUI/BoxesList/boxtargetwidget.cpp \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.cpp \
    GUI/paintboxsettingsdialog.cpp \
    execdelegator.cpp \
    GUI/BoxesList/boxscrollarea.cpp \
    videoencoder.cpp \
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
    GUI/BrushWidgets/brushwidget.cpp

HEADERS  += \
    GUI/BoxesList/OptimalScrollArea/scrollvisiblepartbase.h \
    GUI/BoxesList/boxscroller.h \
    GUI/BrushWidgets/bookmarkedbrushes.h \
    GUI/BrushWidgets/brushlabel.h \
    GUI/ColorWidgets/bookmarkedcolors.h \
    GUI/Dialogs/dialogsinterfaceimpl.h \
    GUI/Dialogs/durationrectsettingsdialog.h \
    GUI/Expressions/expressiondialog.h \
    GUI/Expressions/expressioneditor.h \
    GUI/Expressions/expressionhighlighter.h \
    GUI/RenderWidgets/outputsettingsdialog.h \
    GUI/RenderWidgets/rendersettingsdialog.h \
    GUI/RenderWidgets/rendersettingsdisplaywidget.h \
    GUI/Settings/externalappssettingswidget.h \
    GUI/Settings/performancesettingswidget.h \
    GUI/Settings/settingsdialog.h \
    GUI/Settings/settingswidget.h \
    GUI/Settings/timelinesettingswidget.h \
    GUI/audiohandler.h \
    GUI/bookmarkedwidget.h \
    GUI/buttonbase.h \
    GUI/buttonslist.h \
    GUI/canvasbasewrappernode.h \
    GUI/canvaswrappernode.h \
    GUI/centralwidget.h \
    GUI/changewidthwidget.h \
    GUI/closesignalingdockwidget.h \
    GUI/ecombobox.h \
    GUI/ekeyfilter.h \
    GUI/envelicense.h \
    GUI/envesplash.h \
    GUI/ewidgetsimpl.h \
    GUI/layouthandler.h \
    GUI/scenechooser.h \
    GUI/switchbutton.h \
    GUI/timelinebasewrappernode.h \
    GUI/timelinedockwidget.h \
    GUI/timelinehighlightwidget.h \
    GUI/timelinewidget.h \
    GUI/timelinewrappernode.h \
    GUI/triggerlabel.h \
    GUI/twocolumnlayout.h \
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
    GUI/GradientWidgets/gradientwidget.h \
    GUI/ColorWidgets/glwidget.h \
    GUI/widgetwrappernode.h \
    GUI/wrappernode.h \
    effectsloader.h \
    eimporters.h \
    hardwareinfo.h \
    iconloader.h \
    outputsettings.h \
    renderhandler.h \
    rendersettings.h \
    keypoint.h \
    GUI/BoxesList/OptimalScrollArea/scrollarea.h \
    GUI/BoxesList/OptimalScrollArea/scrollwidget.h \
    GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h \
    GUI/BoxesList/OptimalScrollArea/singlewidget.h \
    GUI/BoxesList/boxsinglewidget.h \
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
    GUI/BoxesList/boxtargetwidget.h \
    GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.h \
    GUI/paintboxsettingsdialog.h \
    execdelegator.h \
    GUI/BoxesList/boxscrollarea.h \
    videoencoder.h \
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

brushesClassic.files = $$files(brushes/Classic/*)
RESOURCES += brushesClassic

brushesDeevad.files = $$files(brushes/Deevad/*)
RESOURCES += brushesDeevad

brushesDieterle.files = $$files(brushes/Dieterle/*)
RESOURCES += brushesDieterle

brushesExperimental.files = $$files(brushes/Experimental/*)
RESOURCES += brushesExperimental

brushesKaerhon.files = $$files(brushes/Kaerhon/*)
RESOURCES += brushesKaerhon

brushesRamon.files = $$files(brushes/Ramon/*)
RESOURCES += brushesRamon

brushesTanda.files = $$files(brushes/Tanda/*)
RESOURCES += brushesTanda

DISTFILES += \
    icons/promoteToLayer \
    icons/toolbarButtons/checkable/color \
    icons/toolbarButtons/checkable/position \
    icons/toolbarButtons/checkable/pressure \
    icons/toolbarButtons/checkable/sculpt+ \
    icons/toolbarButtons/checkable/sculpt- \
    icons/toolbarButtons/checkable/sculpt<- \
    icons/toolbarButtons/checkable/sculpt= \
    icons/toolbarButtons/checkable/sculptUnchecked \
    icons/toolbarButtons/checkable/spacing \
    icons/toolbarButtons/checkable/time \
    icons/toolbarButtons/checkable/width
