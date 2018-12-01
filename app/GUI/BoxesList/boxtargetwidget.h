#ifndef BOXTARGETWIDGET_H
#define BOXTARGETWIDGET_H

#include <QWidget>
class BoundingBox;
class BoxTargetProperty;
#include "sharedpointerdefs.h"

class BoxTargetWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoxTargetWidget(QWidget *parent = nullptr);

    void setTargetProperty(BoxTargetProperty *property);
private:
    bool mDragging = false;
    qptr<BoxTargetProperty> mProperty;
signals:

public slots:
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void paintEvent(QPaintEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void mousePressEvent(QMouseEvent *event);
};

#endif // BOXTARGETWIDGET_H
