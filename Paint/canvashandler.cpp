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

int CanvasHandler::getLayersCount() {
    return layers.count();
}

void CanvasHandler::getTileDrawers(QList<TileSkDrawer*> *tileDrawers) {
    current_layer->getTileDrawers(tileDrawers);
}

void CanvasHandler::drawGridBg() {
    glColor3f(0.2, 0.2, 0.2);
    for(int i = 0; i < width; i += 20) {
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
    layers.append(new Layer(layer_name_t, width, height));
    n_layers++;
}

void CanvasHandler::removeLayer(Layer *layer_t) {
    n_layers--;
    layers.removeOne(layer_t);
}

void CanvasHandler::incNumberItems() {
    n_items++;
    if(!load_img) {
        load_img = true;
    }
}

void CanvasHandler::decNumberItems() {
    n_items--;
    if(n_items == 0) {
        load_img = false;
    }
}

void CanvasHandler::clear() {
    for(int i = 0; i < n_layers; i++) {
        layers.at(i)->clear();
    }
}

void CanvasHandler::paintPress(qreal xT,
                               qreal yT,
                               const ulong &timestamp,
                               const qreal &pressure,
                               Brush *brush) {
    mapToPaintCanvasHandler(&xT, &yT);
    mousePressEvent(xT, yT, timestamp, pressure, brush);
}

void CanvasHandler::tabletEvent(qreal xT,
                                qreal yT,
                                const ulong &time_stamp,
                                const qreal &pressure,
                                const bool &erase) {
    mapToPaintCanvasHandler(&xT, &yT);
    current_layer->tabletEvent(xT, yT,
                               time_stamp,
                               pressure,
                               erase);
}

void CanvasHandler::tabletReleaseEvent() {
    current_layer->tabletReleaseEvent();
}

void CanvasHandler::tabletPressEvent(qreal xT,
                                     qreal yT,
                                     const ulong &time_stamp,
                                     const qreal &pressure,
                                     const bool &erase,
                                     Brush *brush) {
    mapToPaintCanvasHandler(&xT, &yT);
    current_layer->tabletPressEvent(xT, yT,
                                    time_stamp,
                                    pressure,
                                    erase,
                                    brush);
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

void CanvasHandler::mousePressEvent(qreal xT,
                                    qreal yT,
                                    const ulong &timestamp,
                                    const qreal &pressure,
                                    Brush *brush) {
    mapToPaintCanvasHandler(&xT, &yT);
    tabletPressEvent(xT, yT, timestamp, pressure, false, brush);
}

void CanvasHandler::mouseMoveEvent(qreal xT,
                                   qreal yT,
                                   const ulong &time_stamp,
                                   const bool &erase) {
    mapToPaintCanvasHandler(&xT, &yT);
    tabletEvent(xT, yT, time_stamp, 1.0, erase);
}
