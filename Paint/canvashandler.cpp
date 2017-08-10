#include "canvashandler.h"
#include <png++/png.hpp>
#include "PaintLib/surface.h"

void CanvasHandler::setBackgroundMode(
        const CanvasBackgroundMode &bg_mode_t) {
    mBackgroudMode = bg_mode_t;
}

void CanvasHandler::backgroundImgFromFile(
        const QString &file_name) {

}

void CanvasHandler::setBackgroundColorRGB(const qreal &r_t,
                                          const qreal &g_t,
                                          const qreal &b_t) {
    mBackgroundColor.setRGB(r_t, g_t, b_t);
}

void CanvasHandler::setBackgroundColorHSV(const qreal &h_t,
                                          const qreal &s_t,
                                          const qreal &v_t) {
    mBackgroundColor.setHSV(h_t, s_t, v_t);
}

void CanvasHandler::getTileDrawers(
        QList<TileSkDrawer*> *tileDrawers) {
    mPaintlibSurface->getTileDrawers(tileDrawers);
}

void CanvasHandler::setSize(const ushort &widthT,
                            const ushort &heightT) {
    mPaintlibSurface->setSize(widthT, heightT);
}

CanvasHandler::CanvasHandler(const int &width_t,
                             const int &height_t,
                             const qreal &scale,
                             const bool &paintOnOtherThread) {
    mScale = scale;
    mPaintOnOtherThread = paintOnOtherThread;
    mWidth = width_t;
    mHeight = height_t;
    if(width_t != 0) {
        mPaintlibSurface = new Surface(width_t, height_t,
                                       scale,
                                       mPaintOnOtherThread);
    }
}


void CanvasHandler::clear() {
    mPaintlibSurface->clear();
}

void CanvasHandler::paintPress(const qreal &xT,
                               const qreal &yT,
                               const ulong &timestamp,
                               const qreal &pressure,
                               Brush *brush) {
    mousePressEvent(xT, yT, timestamp, pressure, brush);
}

void CanvasHandler::tabletEvent(const qreal &xT,
                                const qreal &yT,
                                const ulong &time_stamp,
                                const qreal &pressure,
                                const bool &erase,
                                Brush *brush) {
    Q_UNUSED(time_stamp);
    mPaintlibSurface->strokeTo(brush,
                               xT, yT,
                               pressure, 100,
                               erase);
}

void CanvasHandler::tabletReleaseEvent() {
}

void CanvasHandler::tabletPressEvent(const qreal &xT,
                                     const qreal &yT,
                                     const ulong &time_stamp,
                                     const qreal &pressure,
                                     const bool &erase,
                                     Brush *brush) {
    mPaintlibSurface->startNewStroke(brush, xT, yT, pressure);
    tabletEvent(xT, yT, time_stamp, pressure, erase, brush);
}

void CanvasHandler::mouseReleaseEvent() {
    tabletReleaseEvent();
}

void CanvasHandler::mousePressEvent(const qreal &xT,
                                    const qreal &yT,
                                    const ulong &timestamp,
                                    const qreal &pressure,
                                    Brush *brush) {
    tabletPressEvent(xT, yT, timestamp, pressure, false, brush);
}

void CanvasHandler::mouseMoveEvent(const qreal &xT,
                                   const qreal &yT,
                                   const ulong &time_stamp,
                                   const bool &erase,
                                   Brush *brush) {
    tabletEvent(xT, yT, time_stamp, 1.0, erase, brush);
}
