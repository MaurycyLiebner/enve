#include "hwheelsvtriangle.h"
#include <QPainter>
#include "mainwindow.h"
#include <QWheelEvent>
#include <QResizeEvent>
#include <QApplication>
#include "Colors/helpers.h"

#define PI 3.14159265
#define RadToF 0.5/PI
#define sqrt_3 sqrt(3)

HWheelSVTriangle::HWheelSVTriangle(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(100, 100);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void HWheelSVTriangle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.drawPixmap(0, 0, wheel_width, wheel_width, wheel_pix);

    p.translate(wheel_width*0.5, wheel_width*0.5);
    p.save();
    p.rotate(hue*360);
    p.translate(-wheel_width*0.5 + 2, 0);
    p.setPen(QPen(MainWindow::color_dark.qcol, 3) );
    p.drawLine(0, 0, wheel_thickness*0.25f, 0);
    p.drawLine(wheel_thickness*0.75f, 0, wheel_thickness - 2, 0);
    p.restore();
    p.rotate(hue*360 + 150);
    float trans_back_t = -(wheel_width*0.5 - wheel_thickness);
    p.translate(trans_back_t, trans_back_t);

    p.drawPixmap(0, 0, triangle_pix);

    float center_y = value*inner_circle_r*1.5;
    float row_width_t = center_y*2/sqrt_3;
    float row_x_0 = inner_circle_r - row_width_t*0.5;
    float center_x = row_x_0 + row_width_t*saturation;
    p.setPen(QPen(Qt::black, 2) );
    p.drawEllipse(QPointF(center_x, center_y), 4, 4);

    p.end();
}

void HWheelSVTriangle::updateWheelPix()
{
    wheel_pix = QPixmap(wheel_width, wheel_width);
    wheel_pix.fill(Qt::transparent);
    QPainter p(&wheel_pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    int outer_circle_r_t = outer_circle_r - 2;
    int bigger_circle_paint_r = outer_circle_r_t;
    int smaller_circle_paint_r = inner_circle_r - 1;
    QColor col_t;
    col_t.setHsvF(1, 1, 1);
    for(int x_t = 0; x_t < wheel_width; x_t++)
    {
        int rel_center_x = x_t - wheel_width*0.5;
        for(int y_t = 0; y_t < wheel_width; y_t++)
        {

            int rel_center_y = y_t - wheel_width*0.5;
            if(insideCircle(bigger_circle_paint_r, rel_center_x, rel_center_y) &&
                    outsideCircle(smaller_circle_paint_r, rel_center_x, rel_center_y) )
            {
                float hue_f = getAngleF(1, 0, rel_center_x, rel_center_y);
                col_t.setHsvF(hue_f, 1, 1 );
                p.setPen(col_t);
                p.drawPoint(x_t, y_t);
            }
        }
    }
    p.setPen(QPen(MainWindow::color_dark.qcol, 4) );
    p.drawEllipse(2, 2, outer_circle_r_t*2, outer_circle_r_t*2);
    p.setPen(Qt::NoPen);
    p.setBrush(MainWindow::color_dark.qcol);
    p.drawEllipse(wheel_thickness, wheel_thickness,
                  inner_circle_r*2, inner_circle_r*2);
    repaint();
}

void HWheelSVTriangle::updateTrianglePix()
{
    float trian_width_t = sqrt_3*inner_circle_r;
    float width_t = inner_circle_r*2;
    float height_t = 1.5*inner_circle_r;
    triangle_pix = QPixmap(width_t, height_t + 2);
    triangle_pix.fill(Qt::transparent);
    QPainter p(&triangle_pix);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    QColor col_t;
    float axis_x = width_t*0.5;
    for(int y_t = 0; y_t < height_t; y_t++)
    {
        float value_t = clamp(y_t*1.0f/height_t, 0.0f, 1.0f);
        float row_width_t = y_t*2/sqrt_3;
        float half_row_width_t = row_width_t*0.5;
        float row_x_min = axis_x - half_row_width_t - 1;
        float row_x_max = axis_x + half_row_width_t + 1;
        for(int x_t = row_x_min; x_t < row_x_max; x_t++)
        {
            float saturation_t = clamp( (x_t - row_x_min)*1.0f/row_width_t, 0.0f, 1.0f );
            col_t.setHsvF(hue, saturation_t, value_t );
            p.setPen(col_t);
            p.drawPoint(x_t, y_t);
        }
    }
    float x_mar = (width_t - trian_width_t)*0.5;
    p.setRenderHint(QPainter::Antialiasing);
    QPointF v1 = QPointF(trian_width_t*0.5 + x_mar, -2);
    QPointF v2 = QPointF(-2 + x_mar, height_t + 2);
    QPointF v3 = QPointF(trian_width_t + 2 + x_mar, height_t + 2);
    p.setPen(QPen(Qt::transparent, 2) );
    p.drawLine(v1, v2);
    p.drawLine(v2, v3);
    p.drawLine(v3, v1);
    p.end();
    repaint();
}

void HWheelSVTriangle::wheelEvent(QWheelEvent *e)
{
    if(e->delta() > 0)
    {
        hue += 0.01;
        if(hue > 1)
        {
            hue -= 1;
        }
    }
    else
    {
        hue -= 0.01;
        if(hue < 0)
        {
            hue += 1;
        }
    }
    updateTrianglePix();
}

void HWheelSVTriangle::resizeEvent(QResizeEvent *e)
{
    wheel_width = std::min(e->size().width(), e->size().height());
    wheel_width -= wheel_width%2;
    outer_circle_r = wheel_width*0.5;
    inner_circle_r = outer_circle_r - wheel_thickness;
    updateTrianglePix();
    updateWheelPix();
}

void HWheelSVTriangle::wheelInteraction(int x_t, int y_t)
{
    double radial_x = x_t - wheel_width*0.5;
    double radial_y = y_t - wheel_width*0.5;
    hue = getAngleF(1, 0, radial_x, radial_y);
    updateTrianglePix();
}

#include <QDebug>
void HWheelSVTriangle::triangleInteraction(int x_t, int y_t)
{
    x_t--;
    y_t--;
    x_t -= wheel_thickness;
    y_t -= wheel_thickness;

    float tr_x_t = x_t - inner_circle_r;
    float tr_y_t = y_t - inner_circle_r;
    float hue_rad = -(hue + 5/12.f)*2*PI;
    rotate(hue_rad, &tr_x_t, &tr_y_t);
    tr_x_t += inner_circle_r;
    tr_y_t += inner_circle_r;
    float row_width_t = tr_y_t*2/sqrt_3;
    float row_x_0 = inner_circle_r - row_width_t*0.5;

    saturation = clamp( (tr_x_t - row_x_0)/row_width_t , 0.f, 1.f);
    value = clamp(tr_y_t/(inner_circle_r*1.5f), 0.f, 1.f);

    repaint();
}

void HWheelSVTriangle::mousePressEvent(QMouseEvent *e)
{
    grabMouse();
    QApplication::setOverrideCursor(QCursor(QPixmap("pixmaps/cursor_crosshair_open.png") ) );
    if(isInTriangle(e->pos() ) )
    {
        value_focus = SV;
        triangleInteraction(e->x(), e->y());
    }
    else if(isInWheel(e->pos()))
    {
        value_focus = H;
        wheelInteraction(e->x(), e->y());
    }
    else
    {
        value_focus = NONE;
    }
}

void HWheelSVTriangle::mouseReleaseEvent(QMouseEvent *e)
{
    releaseMouse();
    QApplication::restoreOverrideCursor();
    if(value_focus == H)
    {
        wheelInteraction(e->x(), e->y());
    }
    else
    {
        triangleInteraction(e->x(), e->y());
    }
}

void HWheelSVTriangle::mouseMoveEvent(QMouseEvent *e)
{
    if(value_focus == H)
    {
        wheelInteraction(e->x(), e->y());
    }
    else if(value_focus == SV)
    {
        triangleInteraction(e->x(), e->y());
    }
}

bool HWheelSVTriangle::isInWheel(QPoint pos_t)
{
    float rel_center_x = pos_t.x() - wheel_width*0.5;
    float rel_center_y = pos_t.y() - wheel_width*0.5;
    return insideCircle(outer_circle_r, rel_center_x, rel_center_y) &&
                        outsideCircle(inner_circle_r, rel_center_x, rel_center_y);
}

bool HWheelSVTriangle::isInTriangle(QPoint pos_t)
{
    float half_width = wheel_width*0.5;
    int outer_circle_r = half_width;
    int inner_circle_r = outer_circle_r - wheel_thickness;
    int trian_width_t = sqrt_3*inner_circle_r;
    int width_t = inner_circle_r*2;
    int height_t = 1.5*inner_circle_r;
    float x_mar = (width_t - trian_width_t)*0.5 + wheel_thickness;
    float y_mar = wheel_thickness;
    float x1_t = trian_width_t*0.5 + x_mar - half_width;
    float y1_t = y_mar - half_width;
    QPointF v1 = QPointF(x1_t, y1_t);
    float x2_t = x_mar - half_width;
    float y2_t = height_t + y_mar - half_width;
    QPointF v2 = QPointF(x2_t, y2_t);
    float x3_t = trian_width_t + x_mar - half_width;
    float y3_t = height_t + y_mar - half_width;
    QPointF v3 = QPointF(x3_t, y3_t);

    float x_t = pos_t.x() - half_width;
    float y_t = pos_t.y() - half_width;
    float hue_rad = -(hue + 5/12.f)*2*PI;
    rotate(hue_rad, &x_t, &y_t);
    return pointInTriangle(x_t, y_t, v1, v2, v3);
}
