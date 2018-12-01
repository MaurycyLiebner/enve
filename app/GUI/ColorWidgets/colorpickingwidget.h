#ifndef COLORPICKINGWIDGET_H
#define COLORPICKINGWIDGET_H

#include <QWidget>

class ColorSettingsWidget;

class ColorPickingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPickingWidget(ColorSettingsWidget *parent = nullptr);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    QColor colorFromPoint(int x_t, int y_t);
    void endThis();
    void updateBox(QPoint pos_t);
signals:

public slots:
private:
    QPixmap mScreenshotPixmap;
    QColor current_color;
    int cursor_x = 0;
    ColorSettingsWidget *mColorSettingsWidget;
    int cursor_y = 0;
};

#endif // COLORPICKINGWIDGET_H
