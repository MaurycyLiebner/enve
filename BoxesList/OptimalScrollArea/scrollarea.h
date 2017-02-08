#ifndef SCROLLAREA_H
#define SCROLLAREA_H
#include <QScrollArea>

class ScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    ScrollArea(QWidget *parent = NULL);

protected:
    int mLastHeight = 0;
    void resizeEvent(QResizeEvent *e);
signals:
    void heightChanged(int);
public slots:
    void callWheelEvent(QWheelEvent *event);
};

#endif // SCROLLAREA_H
