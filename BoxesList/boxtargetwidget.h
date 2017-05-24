#ifndef BOXTARGETWIDGET_H
#define BOXTARGETWIDGET_H

#include <QWidget>
class BoxTargetProperty;

class BoxTargetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxTargetWidget(QWidget *parent = 0);

    void setTargetProperty(BoxTargetProperty *property);
private:
    BoxTargetProperty *mProperty = NULL;
    bool mDragging = false;
signals:

public slots:
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void paintEvent(QPaintEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
};

#endif // BOXTARGETWIDGET_H
