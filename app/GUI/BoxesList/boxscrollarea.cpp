#include "boxscrollarea.h"
#include "GUI/mainwindow.h"
#include "GUI/canvaswindow.h"
BoxScrollArea::BoxScrollArea(QWidget *parent) :
    ScrollArea(parent) {
    setFocusPolicy(Qt::ClickFocus);
}

void BoxScrollArea::focusInEvent(QFocusEvent *) {
    MainWindow::getInstance()->getCanvasWindow()->KFT_setFocus();
}
