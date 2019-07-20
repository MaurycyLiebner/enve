#ifndef COLORPICKINGWIDGET_H
#define COLORPICKINGWIDGET_H

#include <QWidget>

class ColorSettingsWidget;

class ColorPickingWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColorPickingWidget(QWidget * const parent = nullptr);

    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
signals:
    void colorSelected(QColor);
private:
    QColor colorFromPoint(const int x, const int y);
    void endThis();
    void updateBox(const QPoint &pos);

    QImage mScreenshot;
    QColor mCurrentColor;
    int mCursorX = 0;
    int mCursorY = 0;
};

#endif // COLORPICKINGWIDGET_H
