#ifndef SCROLLAREA_H
#define SCROLLAREA_H
#include <QScrollArea>

class ScrollArea : public QScrollArea {
    Q_OBJECT
public:
    ScrollArea(QWidget * const parent = nullptr);

    void scrollBy(const int x, const int y);
    void callWheelEvent(QWheelEvent *event);
signals:
    void heightChanged(int);
    void widthChanged(int);
protected:
    void resizeEvent(QResizeEvent *e);
private:
    int mLastHeight = 0;
    int mLastWidth = 0;
};

#endif // SCROLLAREA_H
