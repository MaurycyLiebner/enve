#ifndef SAVEDCOLORBUTTON_H
#define SAVEDCOLORBUTTON_H

#include <QWidget>
#include <QMouseEvent>
#include "helpers.h"

class SavedColorButton : public QWidget {
    Q_OBJECT
public:
    explicit SavedColorButton(const QColor& colorT,
                              QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
private:
    QColor mColor;

signals:
    void colorButtonPressed(QColor);
public slots:
};

#endif // SAVEDCOLORBUTTON_H
