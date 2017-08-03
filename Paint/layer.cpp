#include "layer.h"
#include <QDebug>
#include "canvashandler.h"
#include "windowvariables.h"
#include "canvas.h"

typedef struct {
    float brush_size;
    float scale;
    const char *brush_file;
} SurfaceTestData;

Layer::Layer(QString layer_name_t, CanvasHandler *canvas_handler_t, WindowVariables *window_vars_t,
             int width_t, int height_t) {
    window_vars = window_vars_t;

    canvas_handler = canvas_handler_t;
    setLayerName(layer_name_t);
    if(width_t != 0)
    {
        setNewPaintLibSurface(width_t, height_t);
    }
}

bool isVisible() { return true; }
bool isSelected() { return true; }

#include <GLES3/gl3.h>
void Layer::paint() {
    if(!isVisible())
    {
        return;
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    paintlib_surface->paintGL();

    if(isSelected() )
    {
        glPushAttrib(GL_ENABLE_BIT);
        glLineStipple(2, 0xAAAA);
        glEnable(GL_LINE_STIPPLE);

        glBegin(GL_LINE_LOOP);
            Color(255, 0 , 0).setGLColor();
            glVertex2f(0.f, 0.f);
            glVertex2f(layer_width, 0.f);
            glVertex2f(layer_width, -layer_height);
            glVertex2f(0.f, -layer_height);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        glPopAttrib();
    }
}

void Layer::clear()
{
    paintlib_surface->clear();
}

void Layer::loadImage(QString img_path)
{
    paintlib_surface->loadImg(img_path);
}

void Layer::setWidth(uint16_t width_t)
{
    layer_width = width_t;
}

void Layer::setHeight(uint16_t height_t)
{
    layer_height = height_t;
}

void Layer::setSize(uint16_t width_t, uint16_t height_t)
{
    layer_height = height_t;
    layer_width = width_t;
}

void Layer::setLayerName(QString layer_name_t)
{
    layer_name = layer_name_t;
}

QString Layer::getLayerName()
{
    return layer_name;
}

void Layer::startStroke(GLfloat x_t, GLfloat y_t, GLfloat pressure)
{
    save_needed = true;
    paintlib_surface->startNewStroke(window_vars->getBrush(), x_t, y_t, pressure);
}

void Layer::setNewPaintLibSurface(int width_t, int height_t)
{
    paintlib_surface = new Surface(width_t, height_t);
    setSize(width_t, height_t);
}

void Layer::repaint()
{
    canvas_handler->repaint();
}

void Layer::savePixelsToPngArray(png::image< png::rgba_pixel_16 > *image)
{
    paintlib_surface->savePixelsToPngArray(image);
}

void Layer::saveLayerIfNeeded()
{
    if(save_needed)
    {
        png::image< png::rgba_pixel_16 > image(layer_width, layer_height);
        savePixelsToPngArray(&image);

        image.write(layer_file_path.toLatin1().toStdString());

        save_needed = false;
    }
}

void Layer::setLayerFilePath(QString path_t)
{
    layer_file_path = path_t;
}

void Layer::renderStroke()
{
    /*if(n_strokes == 0)
    {
        return;
    }
    mypaint_surface_begin_atomic((MyPaintSurface *)mypaint_surface);
    for(int i = 0; i < n_strokes; i ++)
    {
        const Stroke *stroke_t = &strokes.at(i);
        mypaint_brush_stroke_to(window_vars->getBrush(), (MyPaintSurface *)mypaint_surface,
                                stroke_t->x, stroke_t->y, stroke_t->pressure, 0.0f, 0.0f, stroke_t->dt);
    }

    MyPaintRectangle rect;
    mypaint_surface_end_atomic((MyPaintSurface *)mypaint_surface, &rect);*/
    repaint();
    painting = false;//
    //strokes.clear();
    //n_strokes = 0;

}

void Layer::tabletEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, float pressure, bool erase)
{
    Q_UNUSED(time_stamp);
    if(!painting)
    {
        painting = true;
        QTimer::singleShot(time_repaint, this, SLOT(renderStroke()));
    }
    paintlib_surface->strokeTo(window_vars->getBrush(), x_t, y_t, pressure, 100, erase);

    //renderStroke();
    /*Stroke stroke;
    stroke.x = x_t;
    stroke.y = y_t;
    stroke.dt = (time_stamp - last_time_stamp)/( (double)1000);
    stroke.pressure = pressure;
    strokes.append(stroke);
    n_strokes++;*/
}

void Layer::tabletReleaseEvent()
{
    renderStroke();
}

void Layer::tabletPressEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, float pressure)
{
    startStroke(x_t, y_t, pressure);
    tabletEvent(x_t, y_t, time_stamp, pressure, window_vars->erasing);
}

