#include "boxscrollarea.h"

BoxScrollArea::BoxScrollArea(QWidget *parent) :
    ScrollArea(parent) {
    setFocusPolicy(Qt::ClickFocus);
}
#include "mainwindow.h"
#include "canvaswindow.h"
void BoxScrollArea::focusInEvent(QFocusEvent *) {
    MainWindow::getInstance()->getCanvasWindow()->KFT_setFocus();
}
