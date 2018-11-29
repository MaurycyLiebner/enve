#ifndef SURFACE_H
#define SURFACE_H
#include <stdlib.h>
#include "tile.h"
#include "brush.h"
#include "Colors/helpers.h"
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
    char *mData = nullptr;
};

class Surface {
public:
    Surface(const ushort &width_t,
            const ushort &heightT,
            const qreal &scale,
            const bool &paintInOtherThread);
    virtual ~Surface();
    void strokeTo(const Brush *brush,
                  qreal x, qreal y,
                  const qreal &pressure,
                  const ushort &dt,
                  const bool &erase);
    void startNewStroke(const Brush *brush,
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

    virtual void getTileDrawers(QList<TileSkDrawerCollection> *tileDrawers);

    void drawSk(SkCanvas *canvas, SkPaint *paint) {
        canvas->scale(qrealToSkScalar(1./mScale),
                      qrealToSkScalar(1./mScale));
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
                    const Brush *brush);
    void tabletMoveEvent(const qreal &xT,
                     const qreal &yT,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase,
                     const Brush *brush);
    void tabletReleaseEvent();
    virtual void tabletPressEvent(const qreal &xT,
                          const qreal &yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          const Brush *brush);
    void mouseReleaseEvent();
    void mousePressEvent(const qreal &xT,
                         const qreal &yT,
                         const ulong &timestamp,
                         const qreal &pressure,
                         const Brush *brush);
    void mouseMoveEvent(const qreal &xT,
                        const qreal &yT,
                        const ulong &time_stamp,
                        const bool &erase,
                        const Brush *brush);
    void writeSurface(QIODevice *target);
    void readSurface(QIODevice *target);

    void duplicateTilesContentFrom(TilesData *tilesSrc);

    TilesData *getTilesData() {
        return mCurrentTiles.get();
    }
    virtual void move(const int &xT, const int &yT);

    virtual void loadFromImage(const QImage &img);
    void setPickedUpRGBA(qreal red_t,
                         qreal green_t,
                         qreal blue_t,
                         qreal alpha_t);
    void getPickedUpRGBA(qreal *red_t,
                         qreal *green_t,
                         qreal *blue_t,
                         qreal *alpha_t) const;
protected:
    void addPickedUpRGBAFromNewStroke(qreal red_t,
                                      qreal green_t,
                                      qreal blue_t,
                                      qreal alpha_t, const Brush *brush);
    void resetPickedUpRGBA(const Brush *brush);

    void getTileIdsOnRect(const qreal &x_min,
                          const qreal &x_max,
                          const qreal &y_min,
                          const qreal &y_max,
                          int *tile_x_min,
                          int *tile_x_max,
                          int *tile_y_min,
                          int *tile_y_max);
    virtual void currentDataModified() {}

    bool mIsTemporary = false;

    CanvasBackgroundMode mBackgroudMode =
            CANVAS_BACKGROUND_COLOR;
    QColor mBackgroundColor = QColor(255, 255, 255);

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

    int mWidth = 0;
    int mHeight = 0;
    int mNTileCols = 0;
    int mNTileRows = 0;

    qreal picked_up_red = 0.;
    qreal picked_up_green = 0.;
    qreal picked_up_blue = 0.;
    qreal picked_up_alpha = 0.;

    TilesDataSPtr mCurrentTiles;
};

#endif // SURFACE_H
