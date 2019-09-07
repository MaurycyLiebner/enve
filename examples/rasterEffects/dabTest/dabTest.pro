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
# Project created by QtCreator 2019-06-02T11:49:48
#
#-------------------------------------------------

QT += core qml
CONFIG += c++14
ENVE_FOLDER = $$PWD/../../..

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

TARGET = dabTest
TEMPLATE = lib

DEFINES += DABTEST_LIBRARY

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
        dabtest.cpp

HEADERS += \
        dabtest.h \
        dabtest_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    normal.frag

RESOURCES += \
    resources.qrc
