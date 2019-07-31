#ifndef BOXESLISTACTIONBUTTON_H
#define BOXESLISTACTIONBUTTON_H

#include <QWidget>

class BoxesListActionButton : public QWidget {
    Q_OBJECT
public:
    BoxesListActionButton(QWidget * const parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    bool mHover = false;
signals:
    void pressed();
};
#include <functional>
class PixmapActionButton : public BoxesListActionButton {
public:
    PixmapActionButton(QWidget * const parent = nullptr) :
        BoxesListActionButton(parent) {}

    void setPixmapChooser(const std::function<QPixmap*()>& func) {
        mPixmapChooser = func;
    }
protected:
    void paintEvent(QPaintEvent*);
private:
    std::function<QPixmap*()> mPixmapChooser;
};


#endif // BOXESLISTACTIONBUTTON_H
