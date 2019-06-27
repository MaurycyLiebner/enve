#ifndef CHANGEWIDTHWIDGET_H
#define CHANGEWIDTHWIDGET_H
#include <QWidget>

class ChangeWidthWidget : public QWidget {
    Q_OBJECT
public:
    ChangeWidthWidget(QWidget *parent = nullptr);

    void updatePos();

    void paintEvent(QPaintEvent *);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    void enterEvent(QEvent *);

    void leaveEvent(QEvent *);
    int getCurrentWidth() {
        return mCurrentWidth;
    }

signals:
    void widthSet(int);
private:
    bool mHover = false;
    bool mPressed = false;
    int mCurrentWidth = 400;
    int mPressX;
};


#endif // CHANGEWIDTHWIDGET_H
