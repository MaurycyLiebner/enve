#ifndef TILE_H
#define TILE_H

#define TILE_DIM 64

#include <QList>
#include <stdlib.h>
#include <GL/gl.h>
#include "Colors/helpers.h"
#include <png++/png.hpp>
#include "skiaincludes.h"
#include "updatable.h"

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
class Tile;
struct TileSkDrawer : public Updatable {
    TileSkDrawer(Tile *parentTileT,
                 const ushort &xT, const ushort &yT);
    ~TileSkDrawer();

    void drawSk(SkCanvas *canvas, SkPaint *paint = NULL) const;

    void processUpdate();

    void schedulerProccessed();

    void afterUpdate();

    void clearImg();

    Tile *parentTile;
    QList<Dab> dabsToPaint;
    uchar *data;
    sk_sp<SkImage> tileImg;
    ushort posX;
    ushort posY;
    ushort maxPaintY = 0;
    ushort maxPaintX = 0;
};
#include "Colors/color.h"
struct TileSkDrawerCollection {
    TileSkDrawerCollection() {}
//    ~TileSkDrawerCollection();

    void drawSk(SkCanvas *canvas) const {
        if(hueChange) {
            SkPaint paint;
            paint.setAlpha(alpha);
            paint.setBlendMode(SkBlendMode::kDstIn);
            canvas->saveLayer(NULL, NULL);
            canvas->clear(hueCol);
            foreach(TileSkDrawer *drawer, drawers) {
                drawer->drawSk(canvas, &paint);
            }
            canvas->restore();
        } else {
            SkPaint paint;
            paint.setAlpha(alpha);
            foreach(TileSkDrawer *drawer, drawers) {
                drawer->drawSk(canvas, &paint);
            }
        }
    }

    void addTileSkDrawer(TileSkDrawer *drawer) {
        drawers << drawer;
    }

    void setHueChangeEnabled(const bool &bT) {
        hueChange = bT;
    }

    void setHue(const qreal &hueT) {
        Color col;
        col.setHSV(hueT, 1., 1.);
        hueCol = col.getSkColor();
    }
//    void processUpdate();

//    void schedulerProccessed();

//    void afterUpdate();

//    void clearImg();

    QList<TileSkDrawer*> drawers;
    bool hueChange = false;
    SkColor hueCol;
    uchar alpha = 255;
};

class Tile {
public:
    Tile(const ushort &x_t, const ushort &y_t,
         const bool &paintInOtherThread);
    ~Tile();

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
    TileSkDrawer *getTexTileDrawer();

    void addScheduler();

    void setDabsForDrawer();

    void drawSk(SkCanvas *canvas, SkPaint *paint = NULL);

    void saveToTmp();
    void clearTmp();
    void writeTile(QIODevice *target);
    void readTile(QIODevice *target);

    void duplicateFrom(Tile *tile);

    uchar *getData() { return mData; }

    void initializeEmptyTileData();
    void initializeTileData();
    void initializeDrawer();

    void clearTileData();

    void replaceData(const ushort &srcX, const ushort &srcY,
                     const ushort &targetX, const ushort &targetY,
                     const ushort &width, const ushort &height,
                     Tile *srcTile) {
        if(mPaintInOtherThread) {
            if(!mDrawer->finished() ||
                !srcTile->getTexTileDrawer()->finished()) {
                return;
            }
        }
        uchar *srcData = srcTile->getData();
        for(int i = 0; i < width; i++) {
            for(int j = 0; j < height; j++) {
                int idTarget = ((j + targetY)*TILE_DIM + targetX + i)*4;
                int idSrc = ((j + srcY)*TILE_DIM + srcX + i)*4;

                if(idSrc >= TILE_DIM*TILE_DIM*4) {
                    qDebug() << "error";
                }
                if(idTarget >= TILE_DIM*TILE_DIM*4) {
                    qDebug() << "error";
                }
                mData[idTarget] = srcData[idSrc];
                mData[idTarget + 1] = srcData[idSrc + 1];
                mData[idTarget + 2] = srcData[idSrc + 2];
                mData[idTarget + 3] = srcData[idSrc + 3];
            }
        }

        updateDrawerFromDataArray();
    }

    void clearData(const ushort &targetX, const ushort &targetY,
                   const ushort &width, const ushort &height) {
        if(mPaintInOtherThread) {
            if(!mDrawer->finished()) {
                return;
            }
        }
        for(int i = 0; i < width; i++) {
            for(int j = 0; j < height; j++) {
                int idTarget = ((j + targetY)*TILE_DIM + targetX + i)*4;
                mData[idTarget] = 0;
                mData[idTarget + 1] = 0;
                mData[idTarget + 2] = 0;
                mData[idTarget + 3] = 0;
            }
        }

        updateDrawerFromDataArray();
    }
    void finishSettingPixels();
    void setPixel(const int &x, const int &y,
                  const uchar &r, const uchar &g,
                  const uchar &b, const uchar &a);
private:
    bool mPaintInOtherThread;

    void updateDrawerFromDataArray();
    std::shared_ptr<TileSkDrawer> mDrawer;
    SkBitmap mDataTileImage;
    SkBitmap mTmpDataTileImage;
    uchar *mDataTmp = NULL;
    uchar *mData = NULL;

    QList<Dab> mDabsToPaint;

    void setPosInSurface(const ushort &x_t, const ushort &y_t);

    ushort mPosX = 0;
    ushort mPosY = 0;

    ushort mMaxPaintY = 0;
    ushort mMaxPaintX = TILE_DIM;
};

#endif // TILE_H
