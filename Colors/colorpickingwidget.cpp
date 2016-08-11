#include "colorpickingwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

ColorPickingWidget::ColorPickingWidget(QWidget *parent)
    : QWidget(parent)
{
    QPixmap picker("pixmaps/cursor_color_picker.png");
    QApplication::setOverrideCursor(QCursor(picker) );
    grabMouse();
    grabKeyboard();
    setMouseTracking(true);
    setAttribute( Qt::WA_TranslucentBackground, true );
    setStyleSheet("QWidget{background-color: transparent;}");
    showFullScreen();
    updateBox(QCursor::pos());
}

void ColorPickingWidget::mousePressEvent(QMouseEvent *e)
{
    QPoint pos_t = mapToGlobal(e->pos());
    QColor pickedColor = colorFromPoint(pos_t.x(), pos_t.y());
    endThis();
}

void ColorPickingWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(cursor_x + 16, cursor_y + 16, 28, 28, Qt::black);
    p.fillRect(cursor_x + 18, cursor_y + 18, 24, 24, Qt::white);
    p.fillRect(cursor_x + 20, cursor_y + 20, 20, 20, current_color);
    p.end();
}

void ColorPickingWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->isAutoRepeat() )
    {
        return;
    }
    endThis();
}

void ColorPickingWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPoint pos_t = mapToGlobal(e->pos() );
    updateBox(pos_t);
}

QColor ColorPickingWidget::colorFromPoint(int x_t, int y_t)
{
    QPixmap *pix = new QPixmap;
    *pix = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId(), x_t, y_t, 1, 1);
    QImage *img = new QImage;
    *img = pix->toImage();
    QRgb b = img->pixel(0, 0);
    delete pix;
    delete img;
    QColor c;
    c.setRgb(b);
    return c;
}

void ColorPickingWidget::endThis()
{
    QApplication::restoreOverrideCursor();
    releaseMouse();
    releaseKeyboard();
    delete this;
}

void ColorPickingWidget::updateBox(QPoint pos_t)
{
    cursor_x = pos_t.x();
    cursor_y = pos_t.y();
    current_color = colorFromPoint(cursor_x, cursor_y);
    repaint();
}

