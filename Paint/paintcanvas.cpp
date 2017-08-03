#include "paintcanvas.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include "windowvariables.h"
#include "canvashandler.h"
#include <QPainter>

PaintCanvas::PaintCanvas(ushort canvas_width_t, ushort canvas_height_t, bool main_canvas_t,
               WindowVariables *window_vars_t, QWidget *parent) : QOpenGLWidget(parent)
{
    main_canvas = main_canvas_t;
    canvas_width = canvas_width_t;
    canvas_height = canvas_height_t;
    window_vars = window_vars_t;
    centerVisibleRectOn(0.0f, 0.0f);
}

PaintCanvas::~PaintCanvas()
{

}

PaintCanvasRect PaintCanvas::getVisibleRect()
{
    return visible_rect;
}

GLfloat PaintCanvas::getVisibleX()
{
    return visible_rect.x;
}

GLfloat PaintCanvas::getVisibleY()
{
    return visible_rect.y;
}

GLfloat PaintCanvas::getVisibleWidth()
{
    return visible_rect.width;
}

GLfloat PaintCanvas::getVisibleHeight()
{
    return visible_rect.height;
}

void PaintCanvas::moveVisibleRect(GLfloat dx, GLfloat dy)
{
    visible_rect.x += dx;
    visible_rect.y += dy;
    update();
}

void PaintCanvas::centerVisibleRectOn(GLfloat x_c_t, GLfloat y_c_t)
{
    setVisbilePos(x_c_t - width()*0.5/scale, y_c_t - height()*0.5/scale);
}

void PaintCanvas::setVisbilePos(GLfloat x_t, GLfloat y_t)
{
    visible_rect.x = x_t;
    visible_rect.y = y_t;
    update();
}

void PaintCanvas::updateVisibleRectSize()
{
    visible_rect.width = width()/scale;
    visible_rect.height = height()/scale;
    update();
}

void PaintCanvas::mapFromWidgetToPaintCanvasHandler(GLfloat *x_t, GLfloat *y_t)
{
    *x_t = *x_t/scale + visible_rect.x;
    *y_t = *y_t/scale + visible_rect.y;
}

void PaintCanvas::mapFromWidgetToPaintCanvas(GLfloat *x_t, GLfloat *y_t)
{
    *x_t = *x_t/scale + visible_rect.x;
    *y_t = *y_t/scale + visible_rect.y;
}

void PaintCanvas::clear()
{
    canvas_handler->clear();
    repaint();
}

void PaintCanvas::loadImage(int id_t, QString img_path)
{
    if(initialized)
    {
        canvas_handler->loadImage(id_t, img_path);
    }
    else
    {
        img_load_queued = true;
        queued_load_layer_id = id_t;
        queued_load_img = img_path;
    }
}

void PaintCanvas::resetZoom()
{
    scale = 1.f;
    updateVisibleRectSize();
    setVisbilePos(0.f, 0.f);
}

void PaintCanvas::initializeGL()
{
    glClearColor(0.4, 0.4, 0.4, 1);
    glOrtho(0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f);
    canvas_handler = new CanvasHandler(window_vars, canvas_width, canvas_height, main_canvas);

    canvas_handler->newLayer("layer_1");
    canvas_handler->setCurrentLayer(0);

    initialized = true;

    if(img_load_queued)
    {
        loadImage(queued_load_layer_id, queued_load_img);
    }
}

void PaintCanvas::paintGL()
{
    glOrthoAndViewportSet(width(), height());
    Color(180, 180, 180).setGLClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix(); // without transformation
    glScalef(scale, scale, 0.0f);

    glTranslatef(-visible_rect.x, visible_rect.y, 0.0f);

    canvas_handler->paintGL();

    glPopMatrix(); // without transformation
}

void PaintCanvas::moveVisiblePos(GLfloat dx, GLfloat dy)
{
    setVisbilePos(press_v_r_x + dx,
                  press_v_r_y + dy);
}

#include <QApplication>
// only mouse move event

void PaintCanvas::saveMoveEventVars(int x_t, int y_t)
{
    m_track_x = x_t;
    m_track_y = y_t;
    press_v_r_x = visible_rect.x;
    press_v_r_y = visible_rect.y;
}

void PaintCanvas::updateCursor(int radius)
{
    if(radius < 4)
    {
        radius = 4;
    }
    int radius_2 = radius*2;
    QPixmap pix = QPixmap(radius_2 + 5, radius_2 + 5);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::black);
    pen.setWidthF(2.5f);
    p.setPen(pen);
    p.drawEllipse(QRectF(1, 1, radius_2 - 2, radius_2 - 2) );
    pen.setWidthF(1.f);
    pen.setColor(Qt::white);
    p.setPen(pen);
    p.drawEllipse(1, 1, radius_2 - 2, radius_2 - 2);
    setCursor(QCursor(pix));
}

bool PaintCanvas::isZoomingEnabled()
{
    return zooming_enabled;
}

bool PaintCanvas::isMovingEnabled()
{
    return moving_enabled;
}

void PaintCanvas::setZoomingEnabled(bool enabled_t)
{
    zooming_enabled = enabled_t;
}

void PaintCanvas::setMovingEnabled(bool enabled_t)
{
    moving_enabled = enabled_t;
}

void PaintCanvas::paintPress(GLfloat x_t,
                             GLfloat y_t,
                             ulong timestamp,
                             float pressure) {
    GLfloat x_tmp = x_t;
    GLfloat y_tmp = y_t;
    mapFromWidgetToPaintCanvas(&x_t, &y_t);
    mapFromWidgetToPaintCanvasHandler(&x_tmp, &y_tmp);
    canvas_handler->mousePressEvent(x_tmp, y_tmp, timestamp, pressure);
}

void PaintCanvas::mouseMoveEvent(QMouseEvent *e) {
    if(stylus_drawing) {
        return;
    }
    else if(middle_button_pressed && moving_enabled) {
        moveVisiblePos((m_track_x - e->x())/scale,
                       (m_track_y - e->y())/scale);
    } else {
        GLfloat x_t = e->x();
        GLfloat y_t = e->y();
        mapFromWidgetToPaintCanvasHandler(&x_t, &y_t);
        canvas_handler->mousePaintEvent(x_t, y_t, e->timestamp(),
                                        window_vars->erasing);
    }
}


// also tablets press
void PaintCanvas::mousePressEvent(QMouseEvent *e) {
    if(stylus_drawing || middle_button_pressed) {
        return;
    }
    if(e->button() == Qt::RightButton) {

    }
    else if(e->button() == Qt::LeftButton) {
        GLfloat x_t = e->x();
        GLfloat y_t = e->y();
        paintPress(x_t, y_t, e->timestamp(), 0.5);
    } else if(e->button() == Qt::MiddleButton && moving_enabled) {
        middle_button_pressed = true;
        saveMoveEventVars(e->x(), e->y());
    }
}

// also tablets release
void PaintCanvas::mouseReleaseEvent(QMouseEvent *e) {
    if(e->button() == Qt::MiddleButton && moving_enabled) {
        middle_button_pressed = false;
    }
    canvas_handler->mouseReleaseEvent();
}

void PaintCanvas::tabletEvent(QTabletEvent *e) {
    if(e->type() == QEvent::TabletPress) {
        if(e->button() == Qt::RightButton || middle_button_pressed) {
            return;
        } else if(e->button() == Qt::LeftButton) {
            stylus_drawing = true;
            QPoint global_pos = mapToGlobal( QPoint(0, 0) );
            GLfloat w_x_t = e->hiResGlobalX() - global_pos.x();
            GLfloat w_y_t = e->hiResGlobalY() - global_pos.y();
            paintPress(w_x_t, w_y_t, e->timestamp(), e->pressure());
        }
    } else if(e->type() == QEvent::TabletRelease) {
        if(e->button() == Qt::LeftButton) {
            stylus_drawing = false;
            canvas_handler->mouseReleaseEvent();
            update();
        }
    } else if(!middle_button_pressed && stylus_drawing) {
        bool erase = window_vars->erasing;
        if( e->pointerType() == QTabletEvent::Eraser) {
            erase = true;
        }
        QPoint global_pos = mapToGlobal( QPoint(0, 0) );
        GLfloat w_x_t = e->hiResGlobalX() - global_pos.x();
        GLfloat w_y_t = e->hiResGlobalY() - global_pos.y();

        mapFromWidgetToPaintCanvasHandler(&w_x_t, &w_y_t);
        canvas_handler->tabletEvent(w_x_t, w_y_t,
                                    e->timestamp(),
                                    e->pressure(), erase);
    } // else if
}

void PaintCanvas::zoomCenter(GLfloat ds)
{
    if(scale > 10.0f && ds > 0)
    {
        return;
    }
    else if(scale < 0.1f && ds < 0)
    {
        return;
    }
    scale *= 1 + ds;

    GLfloat center_x = width()*0.5f;
    GLfloat center_y = height()*0.5f;

    // zoom under mouse
    setVisbilePos( visible_rect.x + center_x*ds/scale,
                   visible_rect.y + center_y*ds/scale );
    updateVisibleRectSize();
    update();
}

void PaintCanvas::zoom(GLfloat ds, GLfloat center_x, GLfloat center_y)
{
    if(scale > 10.0f && ds > 0)
    {
        return;
    }
    else if(scale < 0.1f && ds < 0)
    {
        return;
    }
    scale *= 1 + ds;

    // zoom under mouse
    setVisbilePos( visible_rect.x + center_x*ds/scale,
                   visible_rect.y + center_y*ds/scale );
    updateVisibleRectSize();
    update();
}

void PaintCanvas::wheelEvent(QWheelEvent *e)
{
    if(zooming_enabled)
    {
        GLfloat ds = e->delta()*0.0005f;
        zoom(ds, e->x(), e->y());
    }
}

void PaintCanvas::resizeGL(int w, int h)
{
    glOrthoAndViewportSet(w, h);
    updateVisibleRectSize();
}

void PaintCanvas::replaceLayerImage(int layer_id_t, QString img_path_t)
{
    canvas_handler->replaceLayerImage(layer_id_t, img_path_t);
}

int PaintCanvas::getLayersCount()
{
    return canvas_handler->getLayersCount();
}

void PaintCanvas::saveAsPng(QString file_name)
{
    canvas_handler->saveAsPng(file_name);
}
