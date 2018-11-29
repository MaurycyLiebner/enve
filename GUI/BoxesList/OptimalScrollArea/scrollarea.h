#ifndef SCROLLAREA_H
#define SCROLLAREA_H
#include <QScrollArea>

class ScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    ScrollArea(QWidget *parent = nullptr);

    void scrollBy(const int &x,
                  const int &y);
protected:
    int mLastHeight = 0;
    int mLastWidth = 0;
    void resizeEvent(QResizeEvent *e);
signals:
    void heightChanged(int);
    void widthChanged(int);
public slots:
    void callWheelEvent(QWheelEvent *event);
};

#endif // SCROLLAREA_H
