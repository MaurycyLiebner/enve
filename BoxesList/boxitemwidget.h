#ifndef BOXITEMWIDGET_H
#define BOXITEMWIDGET_H

#include <QWidget>
#include "connectedtomainwindow.h"
class BoundingBox;

class BoxItemWidget : public QWidget, public ConnectedToMainWindow
{
    Q_OBJECT
public:
    BoxItemWidget(BoundingBox *target, QWidget *parent = 0);

    void paintEvent(QPaintEvent *);

    bool detailsVisible() { return mDetailsVisible; }

    void setDetailsVisibile(bool bT);

    BoundingBox *getTargetBox() { return mTargetBox; }

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void rename();

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int containerTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);
signals:
    void detailsVisibilityChanged(bool);
private:
    BoundingBox *mTargetBox = NULL;
    bool mDetailsVisible = false;
};

#endif // BOXITEMWIDGET_H
