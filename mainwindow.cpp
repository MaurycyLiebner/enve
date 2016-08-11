#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QToolBar>
#include "updatescheduler.h"


Color MainWindow::color_hover = Color(125, 125, 255);
Color MainWindow::color_hover_darker = Color(70, 70, 240);
Color MainWindow::color_selected = Color(255, 125, 125);
Color MainWindow::color_selected_darker = Color(240, 110, 110);
Color MainWindow::color_selected_hover = Color(255, 85, 85);
Color MainWindow::color_selected_hover_darker = Color(240, 70, 70);
Color MainWindow::color_current = Color(255, 175, 125);
Color MainWindow::color_current_darker = Color(255, 160, 95);
Color MainWindow::color_current_hover = Color(255, 130, 50);
Color MainWindow::color_current_hover_darker = Color(255, 115, 25);
Color MainWindow::color_light = Color(225, 225, 225);
Color MainWindow::color_darker = Color(175, 175, 175);
Color MainWindow::color_dark = Color(145, 145, 145);
Color MainWindow::color_selected_active = Color(255, 45, 45);


Color MainWindow::getColor(bool darker, bool selected, bool hover)
{
    if(darker)
    {
        if(selected)
        {
            if(hover)
            {
                return MainWindow::color_selected_hover_darker;
            }
            else
            {
                return MainWindow::color_selected_darker;
            }
        }
        else
        {
            if(hover)
            {
                return MainWindow::color_hover_darker;
            }
            else
            {
                return  MainWindow::color_dark;
            }
        }
    }
    else
    {
        if(selected)
        {
            if(hover)
            {
                return MainWindow::color_selected_hover;
            }
            else
            {
                return MainWindow::color_selected;
            }
        }
        else
        {
            if(hover)
            {
                return MainWindow::color_hover;
            }
            else
            {
                return MainWindow::color_light;
            }
        }
    }
}

QString colorToStyleSheetString(Color color_t)
{
    QString col_s_t = "rgba(%1, %2, %3, %4)";
    QColor qcol_t = color_t.qcol;
    int r_t = qcol_t.red();
    int g_t = qcol_t.green();
    int b_t = qcol_t.blue();
    int a_t = qcol_t.alpha();
    return col_s_t.arg(r_t).arg(g_t).arg(b_t).arg(a_t);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

#include "Colors/ColorWidgets/colorsettingswidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QApplication::instance()->installEventFilter((QObject*)this);
    setStyleSheet(
        "QMainWindow::separator {\
            background: rgb(155, 155, 155);\
            width: 10px;\
            height: 10px;\
        }");

    mCanvas = new Canvas(this);

    mRightDock = new QDockWidget(this);
    mRightDock->setWidget(new ColorSettingsWidget(this));
    mRightDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar	);
    addDockWidget(Qt::RightDockWidgetArea, mRightDock);

    mToolBar = new QToolBar(this);
    mActionConnectPoints = new QAction("CONNECT POINTS", this);
    mToolBar->addAction(mActionConnectPoints);
    connect(mActionConnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(connectPoints()) );

    mActionDisconnectPoints = new QAction("DISCONNECT POINTS", this);
    mToolBar->addAction(mActionDisconnectPoints);
    connect(mActionDisconnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(disconnectPoints()) );

    mActionMergePoints = new QAction("MERGE POINTS", this);
    mToolBar->addAction(mActionMergePoints);
    connect(mActionMergePoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(mergePoints()) );
//
    mActionSymmetricPointCtrls = new QAction("SYMMETRIC POINTS", this);
    mToolBar->addAction(mActionSymmetricPointCtrls);
    connect(mActionSymmetricPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsSymmetric()) );

    mActionSmoothPointCtrls = new QAction("SMOOTH POINTS", this);
    mToolBar->addAction(mActionSmoothPointCtrls);
    connect(mActionSmoothPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsSmooth()) );

    mActionCornerPointCtrls = new QAction("CORNER POINTS", this);
    mToolBar->addAction(mActionCornerPointCtrls);
    connect(mActionCornerPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsCorner()) );
//
    addToolBar(mToolBar);

    setCentralWidget(mCanvas);
}

MainWindow::~MainWindow()
{

}

UndoRedoStack *MainWindow::getUndoRedoStack()
{
    return &mUndoRedoStack;
}

void MainWindow::addUpdateScheduler(UpdateScheduler *scheduler)
{
    mUpdateSchedulers << scheduler;
}

void MainWindow::callUpdateSchedulers()
{
    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        sheduler->update();
        delete sheduler;
    }
    mUpdateSchedulers.clear();
    mCanvas->repaintIfNeeded();
}

void MainWindow::scheduleRepaint()
{
    mCanvas->scheduleRepaint();
}

bool MainWindow::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *key_event = (QKeyEvent*)e;
        return processKeyEvent(key_event);
    }
    return false;
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    bool returnBool = true;
    if(event->key() == Qt::Key_Z &&
            (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
        if(QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            mUndoRedoStack.redo();
        } else {
            mUndoRedoStack.undo();
        }
    } else {
        returnBool = mCanvas->processKeyEvent(event);
    }

    callUpdateSchedulers();
    return returnBool;
}
