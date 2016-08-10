#-------------------------------------------------
#
# Project created by QtCreator 2016-08-06T14:25:20
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AniVect
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    vectorpath.cpp \
    pathpoint.cpp \
    valueanimators.cpp \
    canvas.cpp \
    undoredo.cpp \
    connectedtomainwindow.cpp \
    movablepoint.cpp \
    pathpivot.cpp \
    canvasmouseinteractions.cpp \
    ctrlpoint.cpp \
    boundingbox.cpp

HEADERS  += mainwindow.h \
    vectorpath.h \
    pathpoint.h \
    valueanimators.h \
    canvas.h \
    undoredo.h \
    updatescheduler.h \
    connectedtomainwindow.h \
    movablepoint.h \
    pathpivot.h \
    ctrlpoint.h \
    boundingbox.h
