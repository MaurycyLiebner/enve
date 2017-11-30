#ifndef SURFACE_H
#define SURFACE_H
#include <stdlib.h>
#include "tile.h"
#include "brush.h"
#include "Colors/color.h"
#include <png++/png.hpp>
#include "tilesdata.h"

enum CanvasBackgroundMode {
    CANVAS_BACKGROUND_COLOR,
    CANVAS_BACKGROUND_STRETCHED_IMAGE,
    CANVAS_BACKGROND_REPEATED_IMAGE,
    CANVAS_BACKGROUND_GRID
};

class ImgCharData {
public:
    ImgCharData() {

    }

    ImgCharData(const ushort &width,
                const ushort &height) {
        mWidth = width;
        mHeight = height;
        mData = new char[width*height*4];
    }

    ~ImgCharData() {
        delete[] mData;
    }

    void setSize(const ushort &width,
                 const ushort &height) {
        mWidth = width;
        mHeight = height;
        delete[] mData;
        mData = new char[width*height*4];
    }

    char *getData() { return mData; }
private:
    ushort mWidth = 0;
    ushort mHeight = 0;
    char *mData = NULL;
};

class Surface {
public:
    Surface(const ushort &width_t,
            const ushort &heightT,
            const qreal &scale,
            const bool &paintInOtherThread);
    ~Surface();
    void strokeTo(Brush *brush,
                  qreal x, qreal y,
                  const qreal &pressure,
                  const ushort &dt,
                  const bool &erase);
    void startNewStroke(Brush *brush,
                        qreal x, qreal y,
                        const qreal &pressure);
    void getColor(const qreal &cx,
                  const qreal &cy,
                  const qreal &hardness,
                  const qreal &opa,
                  const qreal &aspect_ratio,
                  const qreal &r,
                  const qreal &beta_deg,
                  qreal *red, qreal *green,
                  qreal *blue, qreal *alpha);

    void clear();

    virtual void getTileDrawers(QList<TileSkDrawer *> *tileDrawers);

    void drawSk(SkCanvas *canvas, SkPaint *paint) {
        canvas->scale(1./mScale, 1./mScale);
        mCurrentTiles->drawSk(canvas, paint);
    }

    void clearTmp();
    void saveToTmp();
    virtual void setSize(const ushort &width_t,
                         const ushort &height_t);
    void paintPress(const qreal &xT,
                    const qreal &yT,
                    const ulong &timestamp,
                    const qreal &pressure,
                    Brush *brush);
    void tabletEvent(const qreal &xT,
                     const qreal &yT,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase,
                     Brush *brush);
    void tabletReleaseEvent();
    virtual void tabletPressEvent(const qreal &xT,
                          const qreal &yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          Brush *brush);
    void mouseReleaseEvent();
    void mousePressEvent(const qreal &xT,
                         const qreal &yT,
                         const ulong &timestamp,
                         const qreal &pressure,
                         Brush *brush);
    void mouseMoveEvent(const qreal &xT,
                        const qreal &yT,
                        const ulong &time_stamp,
                        const bool &erase,
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
    void clearTiles(Tile ***tiles);
    void writeSurface(QIODevice *target);
    void readSurface(QIODevice *target);

    void duplicateTilesContentFrom(Tile ***tilesSrc);

    TilesData *getTilesData() {
        return mCurrentTiles.get();
    }
    virtual void move(const int &xT, const int &yT);

    virtual void loadFromImage(const QImage &img);
protected:
    CanvasBackgroundMode mBackgroudMode =
            CANVAS_BACKGROUND_COLOR;
    Color mBackgroundColor = Color(1.f, 1.f, 1.f);

    qreal mScale = 1.;
    qreal countDabsTo(const qreal &dist_between_dabs,
                      const qreal &x, const qreal &y);

    qreal mPreviousSecondColorAlpha = 0.;
    qreal mNextSecondColorAlpha = 0.;
    uchar mSecondColorAlphaCount = UCHAR_MAX;

    qreal mLastDabRotationInc = 0.;
    qreal mLastStrokeBeta = 0.;

    uchar mStrokeNoiseCount = UCHAR_MAX;
    qreal mPreviousStrokeXNoise = 0.;
    qreal mPreviousStrokeYNoise = 0.;
    qreal mNextStrokeXNoise = 0.;
    qreal mNextStrokeYNoise = 0.;

    uchar mRotationNoiseCount = UCHAR_MAX;
    qreal mPreviousRotationNoise = 0.;
    qreal mNextRotationNoise = 0.;

    uchar mHueNoiseCount = UCHAR_MAX;
    qreal mPreviousHueNoise = 0.;
    qreal mNextHueNoise = 0.;

    uchar mSaturationNoiseCount = UCHAR_MAX;
    qreal mPreviousSaturationNoise = 0.;
    qreal mNextSaturationNoise = 0.;

    uchar mValueNoiseCount = UCHAR_MAX;
    qreal mPreviousValueNoise = 0.;
    qreal mNextValueNoise = 0.;

    qreal mLastXSpeedOffset = 0.;
    qreal mLastYSpeedOffset = 0.;

    qreal mLastEventStrokeX = 0.;
    qreal mLastEventStrokeY = 0.;
    qreal mLastEventStrokeVel = 0.;

    qreal mLastPaintedStrokeX = 0.;
    qreal mLastPaintedStrokeY = 0.;
    qreal mLastStrokePress = 0.;

    ushort mWidth = 0;
    ushort mHeight = 0;
    ushort mNTileCols = 0;
    ushort mNTileRows = 0;
    std::shared_ptr<TilesData> mCurrentTiles;
    void getTileIdsOnRect(const qreal &x_min,
                          const qreal &x_max,
                          const qreal &y_min,
                          const qreal &y_max,
                          short *tile_x_min,
                          short *tile_x_max,
                          short *tile_y_min,
                          short *tile_y_max);
    virtual void currentDataModified() {}
};

#endif // SURFACE_H
