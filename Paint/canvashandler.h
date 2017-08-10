#ifndef CANVASHANDLER_H
#define CANVASHANDLER_H
#include <QList>
#include "Colors/color.h"
class Surface;

enum CanvasBackgroundMode {
    CANVAS_BACKGROUND_COLOR,
    CANVAS_BACKGROUND_STRETCHED_IMAGE,
    CANVAS_BACKGROND_REPEATED_IMAGE,
    CANVAS_BACKGROUND_GRID
};

class Canvas;

class CanvasHandler {
public:
    CanvasHandler(const int &width_t,
                  const int &height_t,
                  const qreal &scale = 1.,
                  const bool &paintOnOtherThread = true);

    void clear();

    void tabletEvent(const qreal &xT,
                     const qreal &yT,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase,
                     Brush *brush);
    void tabletReleaseEvent();
    void tabletPressEvent(const qreal &xT,
                          const qreal &yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          Brush *brush);
    void mouseReleaseEvent();
    void mousePressEvent(const qreal &xT,
                         const qreal &yT,
                         const ulong &timestamp,
                         const qreal &pressure, Brush *brush);
    void mouseMoveEvent(const qreal &xT,
                        const qreal &yT,
                        const ulong &time_stamp,
                        const bool &erase, Brush *brush);
    void paintPress(const qreal &xT,
                    const qreal &yT,
                    const ulong &timestamp,
                    const qreal &pressure,
                    Brush *brush);

    void setBackgroundMode(
            const CanvasBackgroundMode &bg_mode_t);
    void backgroundImgFromFile(const QString &file_name);
    void setBackgroundColorRGB(const qreal &r_t,
                               const qreal &g_t,
                               const qreal &b_t);
    void setBackgroundColorHSV(const qreal &h_t,
                               const qreal &s_t,
                               const qreal &v_t);


    void getTileDrawers(QList<TileSkDrawer*> *tileDrawers);
    void drawSk(SkCanvas *canvas, SkPaint *paint) {
        mPaintlibSurface->drawSk(canvas, paint);
    }

    void clearTmp() {
        mPaintlibSurface->clearTmp();
    }

    void saveToTmp() {
        mPaintlibSurface->saveToTmp();
    }

    void setSize(const ushort &widthT,
                 const ushort &heightT);
private:
    qreal mScale = 1.;
    bool mPaintOnOtherThread = true;

    CanvasBackgroundMode mBackgroudMode =
            CANVAS_BACKGROUND_COLOR;
    Color mBackgroundColor = Color(1.f, 1.f, 1.f);

    int mWidth = 0;
    int mHeight = 0;

    Surface *mPaintlibSurface = NULL;
};

#endif // CANVASHANDLER_H
