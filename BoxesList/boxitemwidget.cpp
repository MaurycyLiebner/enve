#include "boxitemwidget.h"
#include "boxeslistwidget.h"

BoxItemWidget::BoxItemWidget(BoundingBox *target, QWidget *parent) :
    QWidget(parent), ConnectedToMainWindow()
{
    setFixedHeight(BoxesListWidget::getListItemHeight());
    mTargetBox = target;
}

void BoxItemWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setPen(Qt::NoPen);
    if(mTargetBox->isSelected()) {
        p.setBrush(QColor(185, 185, 255));
    } else {
        p.setBrush(QColor(225, 225, 225));
    }
    p.drawRect(rect());

    int drawX = 0;
    if(mDetailsVisible) {
        p.drawPixmap(drawX, 0, *BoxesListWidget::HIDE_CHILDREN);
    } else {
        p.drawPixmap(drawX, 0, *BoxesListWidget::SHOW_CHILDREN);
    }
    drawX += BoxesListWidget::getListItemHeight();
    if(mTargetBox->isVisible()) {
        p.drawPixmap(drawX, 0, *BoxesListWidget::VISIBLE_PIXMAP);
    } else {
        p.drawPixmap(drawX, 0, *BoxesListWidget::INVISIBLE_PIXMAP);
    }
    drawX += BoxesListWidget::getListItemHeight();
    if(mTargetBox->isLocked()) {
        p.drawPixmap(drawX, 0, *BoxesListWidget::LOCKED_PIXMAP);
    } else {
        p.drawPixmap(drawX, 0, *BoxesListWidget::UNLOCKED_PIXMAP);
    }
    drawX += 2*BoxesListWidget::getListItemHeight();
    p.setPen(Qt::black);
    p.drawText(QRect(drawX, 0,
                     width() - drawX -
                     BoxesListWidget::getListItemHeight(),
                     BoxesListWidget::getListItemHeight()),
               mTargetBox->getName(), QTextOption(Qt::AlignVCenter));

    p.end();
}

void BoxItemWidget::setDetailsVisibile(bool bT)
{
    mDetailsVisible = bT;
    emit detailsVisibilityChanged(bT);
}

#include <QInputDialog>
#include <QMenu>
#include "mainwindow.h"
void BoxItemWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->x() < 20) {
        setDetailsVisibile(!mDetailsVisible);
    } else if(event->x() < 40) {
        mTargetBox->setVisibile(!mTargetBox->isVisible());
    } else if(event->x() < 60) {
        mTargetBox->setLocked(!mTargetBox->isLocked());
    } else {
        if(event->button() == Qt::RightButton) {
            if(event->x() >= 60) {
                    QMenu menu(this);


                    menu.addAction("Rename");
                    QAction *selected_action = menu.exec(event->globalPos());
                    if(selected_action != NULL)
                    {
                        if(selected_action->text() == "Rename") {
                            rename();
                        }
                    } else {

                    }
            }
        } else {
            mTargetBox->selectionChangeTriggered(event->modifiers() & Qt::ShiftModifier);
        }
    }
    callUpdateSchedulers();
}

void BoxItemWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->x() >= 60 ) {
        if(e->modifiers() & Qt::ShiftModifier) {
            mousePressEvent(e);
        } else {
            rename();
        }
    } else {
        mousePressEvent(e);
    }
}

void BoxItemWidget::rename() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("New name dialog"),
                                         tr("Name:"), QLineEdit::Normal,
                                         mTargetBox->getName(), &ok);
    if(ok) {
        mTargetBox->setName(text);
        update();
    }
}

void BoxItemWidget::drawKeys(QPainter *p, qreal pixelsPerFrame,
                             int containerTop, int maxY,
                             int minViewedFrame, int maxViewedFrame)
{
    mTargetBox->drawKeys(p, pixelsPerFrame,
                         containerTop,
                         minViewedFrame, maxViewedFrame);
}
