#ifndef TILE_H
#define TILE_H

#define TILE_DIM 64

#include <QList>
#include <stdlib.h>
#include <GL/gl.h>
#include "Colors/helpers.h"
#include <png++/png.hpp>
#include "skiaincludes.h"

struct Dab {
    qreal cx;
    qreal cy;
    qreal hardness;
    qreal opacity;
    qreal aspect_ratio;
    qreal r;
    qreal beta_deg;
    qreal red;
    qreal green;
    qreal blue;
    bool erase;
    Dab(const qreal &cx_t,
        const qreal &cy_t,
        const qreal &hardness_t,
        const qreal &opacity_t,
        const qreal &aspect_ratio_t,
        const qreal &r_t,
        const qreal &beta_t,
        const qreal &red_t,
        const qreal &green_t,
        const qreal &blue_t,
        const bool &eraseT) {
        cx = cx_t;
        cy = cy_t;
        hardness = hardness_t;
        opacity = opacity_t;
        aspect_ratio = aspect_ratio_t;
        r = r_t;
        beta_deg = beta_t;
        red = red_t;
        green = green_t;
        blue = blue_t;
        erase = eraseT;
    }
};

struct TileSkDrawer {
    TileSkDrawer(const sk_sp<SkImage> &img,
                 const ushort &xT, const ushort &yT) {
        tileImg = img;
        posX = xT;
        posY = yT;
    }

    void drawSk(SkCanvas *canvas, SkPaint *paint = NULL) const {
        canvas->drawImage(tileImg, posX, posY, paint);
    }

    sk_sp<SkImage> tileImg;
    ushort posX;
    ushort posY;
};
#include "updatable.h"
class Tile : public Updatable {
public:
    Tile(const ushort &x_t, const ushort &y_t);

    void paintGL();
    void partOfSurfRectInTile(const short &surf_x,
                              const short &surf_y,
                              const short &width,
                              const short &height,
                              short *rect_x, short *rect_y,
                              short *width_t, short *height_t);
    bool surfRectInTile(const int &surf_x,
                        const int &surf_y,
                        const int &width,
                        const int &height);


    void processUpdate();
    void addDabToDraw(qreal cx,
                      qreal cy,
                      const qreal &hardness,
                      const qreal &opa,
                      const qreal &aspect_ratio,
                      const qreal &r,
                      const qreal &beta_deg,
                      const qreal &red,
                      const qreal &green,
                      const qreal &blue,
                      const bool &erase);
    void updateTexFromDataArray();
    void getColor(qreal cx,
                  qreal cy,
                  const qreal &r,
                  const qreal &aspect_ratio,
                  const qreal &cs,
                  const qreal &sn,
                  const qreal &hardness,
                  const qreal &opacity,
                  qreal *red_sum, qreal *green_sum,
                  qreal *blue_sum, qreal *alpha_sum,
                  qreal *weight_sum);

    void clear();

    void setTileWidth(const ushort &width_t);
    void setTileHeight(const ushort &height_t);
    void resetTileSize();
    TileSkDrawer getTexTileDrawer() {
        return TileSkDrawer(mTexTileImage,
                            mPosX, mPosY);
    }

    void schedulerProccessed() {
        mUpdateDabsToPaint = mDabsToPaint;
        mDabsToPaint.clear();
        Updatable::schedulerProccessed();
    }

    void afterUpdate() {
        updateTexFromDataArray();
        Updatable::afterUpdate();
    }

private:
    QList<Dab> mUpdateDabsToPaint;
    sk_sp<SkImage> mDataTileImage;
    uchar *mData = NULL;

    sk_sp<SkImage> mTexTileImage;
    sk_sp<SkImage> mTmpTexOverlay;

    QList<Dab> mDabsToPaint;

    void setPosInSurface(const ushort &x_t, const ushort &y_t);

    ushort mPosX = 0;
    ushort mPosY = 0;

    ushort mMinPaintY = 0;
    ushort mMaxPaintX = TILE_DIM;
};

#endif // TILE_H
