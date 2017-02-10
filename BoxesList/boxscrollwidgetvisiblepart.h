#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
class QrealKey;

class BoxScrollWidgetVisiblePart : public ScrollWidgetVisiblePart
{
    Q_OBJECT
public:
    explicit BoxScrollWidgetVisiblePart(ScrollWidget *parent = 0);


    SingleWidget *createNewSingleWidget();
    void paintEvent(QPaintEvent *);
    void drawKeys(QPainter *p,
                  const qreal &pixelsPerFrame,
                  const int &minViewedFrame,
                  const int &maxViewedFrame);
    QrealKey *getKeyAtPos(const int &pressX,
                          const int &pressY,
                          const qreal &pixelsPerFrame,
                          const int &minViewedFrame);
    void getKeysInRect(QRectF selectionRect,
                       qreal pixelsPerFrame,
                       const int &minViewedFrame,
                       QList<QrealKey *> *listKeys);
signals:

public slots:
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
