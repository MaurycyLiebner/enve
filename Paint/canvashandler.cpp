#include "canvashandler.h"
#include "windowvariables.h"
#include <png++/png.hpp>

void CanvasHandler::setBackgroundMode(CanvasBackgroundMode bg_mode_t)
{
    backgroud_mode = bg_mode_t;
}

void CanvasHandler::backgroundImgFromFile(QString file_name)
{

}

void CanvasHandler::setBackgroundColorRGB(GLfloat r_t, GLfloat g_t, GLfloat b_t)
{
    background_color.setRGB(r_t, g_t, b_t);
}

void CanvasHandler::setBackgroundColorHSV(GLfloat h_t, GLfloat s_t, GLfloat v_t)
{
    background_color.setHSV(h_t, s_t, v_t);
}

bool CanvasHandler::connectedToTreeWidget()
{
    return main_canvas;
}

void CanvasHandler::repaint()
{
    //canvas->repaint();
}

void CanvasHandler::replaceLayerImage(int layer_id_t, QString img_path_t)
{
    Layer *layer_t = layers.at(layer_id_t);
    layer_t->saveLayerIfNeeded();
    QFile file(img_path_t);
    if(file.exists() )
    {
        layer_t->loadImage(img_path_t);
    }
    layer_t->setLayerFilePath(img_path_t);
}

int CanvasHandler::getLayersCount()
{
    return layers.count();
}

void CanvasHandler::drawGridBg()
{
    glColor3f(0.2, 0.2, 0.2);
    for(int i = 0; i < width; i += 20)
    {
        short dx = 0;
        for(int j = 0; j > -height; j -= 10) {
            glRects(i + dx, j, i + 10 + dx, j - 10);
            dx = abs(dx - 10);
        }
    }
}

void CanvasHandler::drawColorBg() {
    background_color.setGLColor();
    glRects(0, 0, width, -height);
}

void CanvasHandler::drawStretchedImgBg()
{

}

void CanvasHandler::drawRepeatedImgBg()
{

}

CanvasHandler::CanvasHandler(WindowVariables *window_vars_t,
                             int width_t, int height_t, bool main_canvas_t)
{
    main_canvas = main_canvas_t;
    window_vars = window_vars_t;
    width = width_t;
    height = height_t;
}

void CanvasHandler::saveAsPng(QString file_name)
{
    png::image< png::rgba_pixel_16 > image(width, height);
    foreach(Layer *layer_t, layers)
    {
        layer_t->savePixelsToPngArray(&image);
    }

    image.write(file_name.toLatin1().toStdString());
}

void CanvasHandler::newLayer(QString layer_name_t)
{
    layers.append(new Layer(layer_name_t, this, window_vars, width, height));
    n_layers++;
}

void CanvasHandler::removeLayer(Layer *layer_t)
{
    n_layers--;
    layers.removeOne(layer_t);
}

void CanvasHandler::paintGL()
{
    glPushMatrix();

    if(backgroud_mode == CANVAS_BACKGROUND_GRID)
    {
        drawGridBg();
    }
    else if(backgroud_mode == CANVAS_BACKGROUND_COLOR)
    {
        drawColorBg();
    }
    else if(backgroud_mode == CANVAS_BACKGROUND_STRETCHED_IMAGE)
    {
        drawStretchedImgBg();
    }
    else if(backgroud_mode == CANVAS_BACKGROND_REPEATED_IMAGE)
    {
        drawRepeatedImgBg();
    }

    for(int i = 0; i < n_layers; i++)
    {
        layers.at(i)->paint();
    }

    glPopMatrix();
}

void CanvasHandler::incNumberItems()
{
    n_items++;
    if(!load_img)
    {
        load_img = true;
    }
}

void CanvasHandler::decNumberItems()
{
    n_items--;
    if(n_items == 0)
    {
        load_img = false;
    }
}

void CanvasHandler::loadImage(int id_t, QString img_path)
{
    layers.at(id_t)->loadImage(img_path);
}

void CanvasHandler::clear()
{
    for(int i = 0; i < n_layers; i++)
    {
        layers.at(i)->clear();
    }
}

void CanvasHandler::tabletEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, float pressure, bool erase)
{
    current_layer->tabletEvent(x_t, y_t, time_stamp, pressure, erase);
}

void CanvasHandler::tabletReleaseEvent()
{
    current_layer->tabletReleaseEvent();
}

void CanvasHandler::tabletPressEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, float pressure)
{
    window_vars->saveBrushAndColorIfNeeded();
    current_layer->tabletPressEvent(x_t, y_t, time_stamp, pressure);
}

void CanvasHandler::setCurrentLayer(int layer_id)
{
    setCurrentLayer(layers.at(layer_id));
}

void CanvasHandler::setCurrentLayer(Layer *layer_t)
{
    current_layer = layer_t;
}

void CanvasHandler::mouseReleaseEvent()
{
    tabletReleaseEvent();
}

void CanvasHandler::mousePressEvent(GLfloat x_t, GLfloat y_t, ulong timestamp, float pressure)
{
    tabletPressEvent(x_t, y_t, timestamp, pressure);
}

void CanvasHandler::mousePaintEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, bool erase)
{
    tabletEvent(x_t, y_t, time_stamp, 1.0, erase);
}
