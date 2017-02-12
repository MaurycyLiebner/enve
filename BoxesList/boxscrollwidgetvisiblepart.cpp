#include "boxscrollwidgetvisiblepart.h"
#include "boxsinglewidget.h"
#include <QPainter>
#include "Animators/qrealanimator.h"

BoxScrollWidgetVisiblePart::BoxScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    ScrollWidgetVisiblePart(parent) {

}

SingleWidget *BoxScrollWidgetVisiblePart::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}

void BoxScrollWidgetVisiblePart::paintEvent(QPaintEvent *) {
    QPainter p(this);

//    p.fillRect(rect(), Qt::red);
    int currY = BOX_HEIGHT;
    p.setPen(QPen(QColor(40, 40, 40), 1.));
    while(currY < height()) {
        p.drawLine(0, currY, width(), currY);

        currY += BOX_HEIGHT;
    }

    p.end();
}

void BoxScrollWidgetVisiblePart::drawKeys(QPainter *p,
             const qreal &pixelsPerFrame,
             const int &minViewedFrame, const int &maxViewedFrame) {
    p->setPen(QPen(Qt::black, 1.));
    foreach(SingleWidget *container, mSingleWidgets) {
        ((BoxSingleWidget*)container)->drawKeys(
                            p, pixelsPerFrame,
                            container->y(),
                            minViewedFrame, maxViewedFrame);
    }
}

QrealKey *BoxScrollWidgetVisiblePart::getKeyAtPos(
        const int &pressX, const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    int remaining = pressY % BOX_HEIGHT;
    if(remaining < (BOX_HEIGHT - KEY_RECT_SIZE)/2 ||
       remaining > (BOX_HEIGHT + KEY_RECT_SIZE)/2) return NULL;
    foreach(SingleWidget *container, mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return ((BoxSingleWidget*)container)->
                getKeyAtPos(pressX, pixelsPerFrame,
                            minViewedFrame);
    }
    return NULL;
}

void BoxScrollWidgetVisiblePart::getKeysInRect(
        QRectF selectionRect,
        qreal pixelsPerFrame,
        const int &minViewedFrame,
        QList<QrealKey*> *listKeys) {
    selectionRect.adjust(0, (BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5,
                         0, -(BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5);
    foreach(SingleWidget *container, mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > selectionRect.bottom() ||
           containerBottom < selectionRect.top()) continue;
        ((BoxSingleWidget*)container)->
                getKeysInRect(selectionRect,
                              pixelsPerFrame,
                              minViewedFrame,
                              listKeys);
    }
}
