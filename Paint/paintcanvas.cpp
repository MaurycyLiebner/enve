#include "paintcanvas.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include "windowvariables.h"
#include "canvashandler.h"
#include <QPainter>

PaintCanvas::PaintCanvas(const ushort &canvas_width_t,
                         const ushort &canvas_height_t) {
    canvas_handler = new CanvasHandler(canvas_width_t,
                                       canvas_height_t);

    canvas_handler->newLayer("layer_1");
    canvas_handler->setCurrentLayer(0);
}

void PaintCanvas::clear() {
    canvas_handler->clear();
    repaint();
}

#include <QApplication>
// only mouse move event

void PaintCanvas::paintPress(const qreal &x_t,
                             const qreal &y_t,
                             const ulong &timestamp,
                             const qreal &pressure) {
    qreal x_tmp = x_t;
    qreal y_tmp = y_t;
    mapToPaintCanvasHandler(&x_tmp, &y_tmp);
    canvas_handler->mousePressEvent(x_tmp, y_tmp, timestamp, pressure);
}

void PaintCanvas::mouseMoveEvent(QMouseEvent *e) {
    if(mStylusDrawing) {
        return;
    } else {
        qreal x_t = e->x();
        qreal y_t = e->y();
        mapToPaintCanvasHandler(&x_t, &y_t);
        canvas_handler->mousePaintEvent(x_t, y_t, e->timestamp(),
                                        window_vars->erasing);
    }
}


// also tablets press
void PaintCanvas::mousePressEvent(QMouseEvent *e) {
    if(mStylusDrawing) {
        return;
    }
    if(e->button() == Qt::RightButton) {

    } else if(e->button() == Qt::LeftButton) {
        GLfloat x_t = e->x();
        GLfloat y_t = e->y();
        paintPress(x_t, y_t, e->timestamp(), 0.5);
    }
}

// also tablets release
void PaintCanvas::mouseReleaseEvent(QMouseEvent *e) {
    canvas_handler->mouseReleaseEvent();
}

void PaintCanvas::tabletEvent(QTabletEvent *e) {
    if(e->type() == QEvent::TabletPress) {
        if(e->button() == Qt::RightButton) {
            return;
        } else if(e->button() == Qt::LeftButton) {
            mStylusDrawing = true;
            QPoint global_pos = mapToGlobal( QPoint(0, 0) );
            qreal w_x_t = e->hiResGlobalX() - global_pos.x();
            qreal w_y_t = e->hiResGlobalY() - global_pos.y();
            paintPress(w_x_t, w_y_t, e->timestamp(), e->pressure());
        }
    } else if(e->type() == QEvent::TabletRelease) {
        if(e->button() == Qt::LeftButton) {
            mStylusDrawing = false;
            canvas_handler->mouseReleaseEvent();
            update();
        }
    } else if(mStylusDrawing) {
        bool erase = window_vars->erasing;
        if(e->pointerType() == QTabletEvent::Eraser) {
            erase = true;
        }
        QPoint global_pos = mapToGlobal( QPoint(0, 0) );
        GLfloat w_x_t = e->hiResGlobalX() - global_pos.x();
        GLfloat w_y_t = e->hiResGlobalY() - global_pos.y();

        mapToPaintCanvasHandler(&w_x_t, &w_y_t);
        canvas_handler->tabletEvent(w_x_t, w_y_t,
                                    e->timestamp(),
                                    e->pressure(), erase);
    } // else if
}

int PaintCanvas::getLayersCount() {
    return canvas_handler->getLayersCount();
}
