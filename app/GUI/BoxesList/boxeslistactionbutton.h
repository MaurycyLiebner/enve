#ifndef BOXESLISTACTIONBUTTON_H
#define BOXESLISTACTIONBUTTON_H

#include <QWidget>

class BoxesListActionButton : public QWidget
{
    Q_OBJECT
public:
    BoxesListActionButton(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    bool mHover = false;
signals:
    void pressed();
public slots:
};

#endif // BOXESLISTACTIONBUTTON_H
