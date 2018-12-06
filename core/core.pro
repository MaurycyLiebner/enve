#-------------------------------------------------
#
# Project created by QtCreator 2018-12-01T10:47:47
#
#-------------------------------------------------

#QT       -= gui

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
    undoredo.cpp

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
    undoredo.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
