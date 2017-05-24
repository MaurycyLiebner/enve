#include "colorpickingwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include "ColorWidgets/colorsettingswidget.h"
#include <QDebug>
#include "global.h"

ColorPickingWidget::ColorPickingWidget(ColorSettingsWidget *parent)
    : QWidget()
{
    mColorSettingsWidget = parent;
    QPixmap picker(":/cursors/cursor_color_picker.png");
    QApplication::setOverrideCursor(QCursor(picker, 2, 20) );
    grabMouse();
    grabKeyboard();
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
    showFullScreen();
    updateBox(QCursor::pos());
}

void ColorPickingWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton) endThis();
    QPoint pos_t = mapToGlobal(e->pos());
    QColor pickedColor = colorFromPoint(pos_t.x(), pos_t.y());
    Color color;
    color.setQColor(pickedColor);
    mColorSettingsWidget->setCurrentColor(color);
    mColorSettingsWidget->emitFullColorChangedSignal();
    MainWindow::getInstance()->callUpdateSchedulers();
    endThis();
}

void ColorPickingWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(cursor_x + 16, cursor_y + 16, 28, 28, Qt::black);
    p.fillRect(cursor_x + 18, cursor_y + 18, 24, 24, Qt::white);
    p.fillRect(cursor_x + MIN_WIDGET_HEIGHT,
               cursor_y + MIN_WIDGET_HEIGHT,
               MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT,
               current_color);
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
    updateBox(e->pos());
}

QColor ColorPickingWidget::colorFromPoint(int x_t, int y_t)
{
    QPixmap *pix = new QPixmap;
    *pix = QGuiApplication::primaryScreen()->grabWindow(
                QApplication::desktop()->winId(), x_t, y_t, 1, 1);
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
    deleteLater();
}

void ColorPickingWidget::updateBox(QPoint pos_t)
{
    cursor_x = pos_t.x();
    cursor_y = pos_t.y();
    QPointF globalPos = mapToGlobal(pos_t);
    current_color = colorFromPoint(globalPos.x(), globalPos.y());
    update();
}

