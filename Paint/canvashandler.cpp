#include "canvashandler.h"
#include "windowvariables.h"
#include <png++/png.hpp>

void CanvasHandler::setBackgroundMode(CanvasBackgroundMode bg_mode_t) {
    backgroud_mode = bg_mode_t;
}

void CanvasHandler::backgroundImgFromFile(QString file_name) {

}

void CanvasHandler::setBackgroundColorRGB(const qreal &r_t,
                                          const qreal &g_t,
                                          const qreal &b_t) {
    background_color.setRGB(r_t, g_t, b_t);
}

void CanvasHandler::setBackgroundColorHSV(const qreal &h_t,
                                          const qreal &s_t,
                                          const qreal &v_t) {
    background_color.setHSV(h_t, s_t, v_t);
}

void CanvasHandler::repaint() {
    //canvas->repaint();
}

int CanvasHandler::getLayersCount() {
    return layers.count();
}

void CanvasHandler::getTileDrawers(QList<TileSkDrawer> *tileDrawers) {
    current_layer->getTileDrawers(tileDrawers);
}

void CanvasHandler::drawGridBg() {
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

void CanvasHandler::drawStretchedImgBg() {

}

void CanvasHandler::drawRepeatedImgBg() {

}

CanvasHandler::CanvasHandler(int width_t, int height_t) {
    width = width_t;
    height = height_t;
    newLayer("layer_1");
    setCurrentLayer(0);
}

void CanvasHandler::newLayer(QString layer_name_t) {
    layers.append(new Layer(layer_name_t, this, width, height));
    n_layers++;
}

void CanvasHandler::removeLayer(Layer *layer_t) {
    n_layers--;
    layers.removeOne(layer_t);
}

void CanvasHandler::paintGL() {
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

void CanvasHandler::clear() {
    for(int i = 0; i < n_layers; i++)
    {
        layers.at(i)->clear();
    }
}

void CanvasHandler::tabletEvent(const qreal &x_t,
                                const qreal &y_t,
                                ulong time_stamp,
                                float pressure,
                                bool erase) {
    qreal xT = absPos.x();
    qreal yT = absPos.y();
    mapToPaintCanvasHandler(&xT, &yT);
    current_layer->tabletEvent(x_t, y_t,
                               time_stamp,
                               pressure,
                               erase);
}

void CanvasHandler::tabletReleaseEvent() {
    current_layer->tabletReleaseEvent();
}

void CanvasHandler::tabletPressEvent(const qreal &x_t,
                                     const qreal &y_t,
                                     ulong time_stamp,
                                     float pressure) {
    mapToPaintCanvasHandler(&x_t, &y_t);
    current_layer->tabletPressEvent(x_t, y_t,
                                    time_stamp,
                                    pressure);
}

void CanvasHandler::setCurrentLayer(int layer_id)
{
    setCurrentLayer(layers.at(layer_id));
}

void CanvasHandler::setCurrentLayer(Layer *layer_t)
{
    current_layer = layer_t;
}

void CanvasHandler::mouseReleaseEvent() {
    tabletReleaseEvent();
}

void CanvasHandler::mousePressEvent(const qreal &x_t,
                                    const qreal &y_t,
                                    ulong timestamp,
                                    float pressure) {
    mapToPaintCanvasHandler(&x_t, &y_t);
    tabletPressEvent(x_tmp, y_tmp, timestamp, pressure);
}

void CanvasHandler::mouseMoveEvent(const qreal &x_t,
                                    const qreal &y_t,
                                    ulong time_stamp,
                                    bool erase) {
    mapToPaintCanvasHandler(&x_t, &y_t);
    tabletEvent(x_t, y_t, time_stamp, 1.0, erase);
}
