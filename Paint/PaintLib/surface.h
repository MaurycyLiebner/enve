#ifndef SURFACE_H
#define SURFACE_H
#include <stdlib.h>
#include "tile.h"
#include "brush.h"
#include <png++/png.hpp>

class Surface {
public:
    Surface(const ushort &width_t,
            const ushort &height_t,
            const qreal &scale,
            const bool &paintOnOtherThread = true);
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

    void getTileDrawers(QList<TileSkDrawer *> *tileDrawers);

    void drawSk(SkCanvas *canvas, SkPaint *paint) {
        canvas->scale(4., 4.);
        for(int i = 0; i < mNTileCols; i++) {
            for(int j = 0; j < mNTileRows; j++) {
                mTiles[j][i]->drawSk(canvas, paint);
            }
        }
    }

    void clearTmp();
    void saveToTmp();
    void setSize(const ushort &width_t,
                 const ushort &height_t);
private:
    qreal mScale = 1.;
    bool mPaintInOtherThread = true;
    qreal countDabsTo(const qreal &dist_between_dabs,
                      const qreal &x, const qreal &y);
    Tile *getTile(const ushort &tile_col,
                  const ushort &tile_row);

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
    Tile ***mTiles = NULL;
    void getTileIdsOnRect(const qreal &x_min,
                          const qreal &x_max,
                          const qreal &y_min,
                          const qreal &y_max,
                          short *tile_x_min,
                          short *tile_x_max,
                          short *tile_y_min,
                          short *tile_y_max);
};

#endif // SURFACE_H
