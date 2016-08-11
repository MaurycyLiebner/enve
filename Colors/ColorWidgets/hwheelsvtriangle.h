#ifndef HWHEELSVTRIANGLE_H
#define HWHEELSVTRIANGLE_H

#include <QWidget>
#include <QTimer>

enum VALUEFOCUS
{
    H,
    SV,
    NONE
};

class HWheelSVTriangle : public QWidget
{
    Q_OBJECT
public:
    explicit HWheelSVTriangle(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

    void resizeEvent(QResizeEvent *e);

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    bool isInTriangle(QPoint pos_t);
    bool isInWheel(QPoint pos_t);
    void wheelInteraction(int x_t, int y_t);
    void triangleInteraction(int x_t, int y_t);
    void updateWheelPix();

    void wheelEvent(QWheelEvent *e);
    void updateTrianglePix();
signals:

public slots:

private:
    int update_c = 0;
    VALUEFOCUS value_focus = NONE;
    float hue = 0.2f;
    float saturation = 0.5f;
    float value = 0.5f;
    QPixmap wheel_pix;
    QPixmap triangle_pix;
    QPixmap triangle_render;
    int wheel_width = 100;
    float outer_circle_r = 0;
    float inner_circle_r = 0;
    int wheel_thickness = 20;
};

#endif // HWHEELSVTRIANGLE_H
