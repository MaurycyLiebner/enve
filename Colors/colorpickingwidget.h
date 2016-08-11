#ifndef COLORPICKINGWIDGET_H
#define COLORPICKINGWIDGET_H

#include <QWidget>

class ColorPickingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPickingWidget(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    QColor colorFromPoint(int x_t, int y_t);
    void endThis();
    void updateBox(QPoint pos_t);
signals:

public slots:
private:
    QColor current_color;
    int cursor_x = 0;
    int cursor_y = 0;
};

#endif // COLORPICKINGWIDGET_H
