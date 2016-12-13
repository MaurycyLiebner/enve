#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
class Canvas;
class FillStrokeSettingsWidget;

class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(FillStrokeSettingsWidget *fillStrokeSettingsWidget,
                          QWidget *parent = 0);

    Canvas *getCanvas();
protected:
    Canvas *mCanvas;

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
signals:

public slots:
};

#endif // CANVASWIDGET_H
