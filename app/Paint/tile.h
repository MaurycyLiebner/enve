#ifndef TILE_H
#define TILE_H

#define TILE_DIM 64

#include <QList>
#include <stdlib.h>
#include "colorhelpers.h"
#include <png++/png.hpp>
#include <SkColorMatrixFilter.h>
#include "skia/skiaincludes.h"
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

extern void processPaintDabs(const QList<Dab> &dabs,
                             const ushort &maxPaintX,
                             const ushort &maxPaintY,
                             const ushort &dataWidth,
                             uchar *data);

class Tile;
struct TileSkDrawer : public _ScheduledTask {
    TileSkDrawer(Tile *parentTileT,
                 const ushort &xT, const ushort &yT);
    ~TileSkDrawer();

    void drawSk(SkCanvas *canvas, SkPaint *paint = nullptr) const;

    void _processUpdate();

    void taskQued();

    void beforeProcessingStarted() {
        _ScheduledTask::beforeProcessingStarted();
        _maxPaintX = maxPaintX;
        _maxPaintY = maxPaintY;
    }
    void afterProcessingFinished();

    void clearImg();

    Tile *parentTile;
    QList<Dab> _dabsToPaint;
    uchar *_data;
    sk_sp<SkImage> tileImg;
    ushort posX;
    ushort posY;
    ushort maxPaintY = TILE_DIM;
    ushort maxPaintX = TILE_DIM;
    ushort _maxPaintY = 0;
    ushort _maxPaintX = 0;
};

struct TileSkDrawerCollection {
    TileSkDrawerCollection() {}
//    ~TileSkDrawerCollection();

    void drawSk(SkCanvas *canvas) const {
        QRectF rect;
        for(TileSkDrawer *drawer : drawers) {
            if(rect.isNull()) {
                rect = QRectF(drawer->posX,
                              drawer->posY,
                              TILE_DIM, TILE_DIM);
                continue;
            }
            rect = rect.united(QRectF(drawer->posX,
                               drawer->posY,
                               TILE_DIM, TILE_DIM));
        }
        SkBitmap bitmap;
        SkImageInfo info = SkImageInfo::Make(qCeil(rect.width()),
                                             qCeil(rect.height()),
                                             kBGRA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr);
        bitmap.allocPixels(info);
        SkCanvas *bitmapCanvas = new SkCanvas(bitmap);
        bitmapCanvas->clear(SK_ColorTRANSPARENT);
        if(hueChange) {
            SkPaint paint;
            paint.setAlpha(alpha);
            paint.setColorFilter(SkColorMatrixFilter::MakeLightingFilter(
                                     SkColorSetARGB(255,
                                                  SkColorGetR(hueCol)*0.7,
                                                  SkColorGetG(hueCol)*0.7,
                                                  SkColorGetB(hueCol)*0.7),
                                     SkColorSetARGB(255,
                                                  SkColorGetR(hueCol)*0.3,
                                                  SkColorGetG(hueCol)*0.3,
                                                  SkColorGetB(hueCol)*0.3)));
//            paint.setBlendMode(SkBlendMode::kDstIn);
//            canvas->saveLayer(nullptr, nullptr);
//            canvas->clear(hueCol);
            for(TileSkDrawer *drawer : drawers) {
                drawer->drawSk(bitmapCanvas, &paint);
            }
            //canvas->restore();
        } else {
            SkPaint paint;
            paint.setAlpha(alpha);
            for(TileSkDrawer *drawer : drawers) {
                drawer->drawSk(bitmapCanvas, &paint);
            }
        }
        bitmapCanvas->flush();
        delete bitmapCanvas;
        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality);
        paint.setAntiAlias(true);
        canvas->drawBitmap(bitmap, 0.f, 0.f, &paint);
    }

    void addTileSkDrawer(TileSkDrawer *drawer) {
        drawers << drawer;
    }

    void setHueChangeEnabled(const bool &bT) {
        hueChange = bT;
    }

    void setHue(const qreal &hueT);
//    void processUpdate();

//    void schedulerProccessed();

//    void afterUpdate();

//    void clearImg();

    QList<TileSkDrawer*> drawers;
    bool hueChange = false;
    SkColor hueCol;
    uchar alpha = 255;
};

//class TileExecutor : public GUI_ThreadExecutor {
//public:
//    TileExecutor(Tile *tile) {
//        mTarget = tile;
//    }
//protected:
//    Tile *mTarget = nullptr;
//};

//class TileClearExecutor : public TileExecutor {
//public:
//    TileClearExecutor(Tile *target) : TileExecutor(target) {
//    }

//protected:
//    void GUI_process() {
//        mTarget->clear();
//    }
//};

//class TileDataGetterDependentExecutor : public TileExecutor {
//public:
//    TileDataGetterDependentExecutor(Tile *src, Tile *target) :
//        TileExecutor(target) {
//        mSrcGetter = SPtrCreate(TileDataGetter)(src);
//    }

//    void dataReady(uchar *data, const sk_sp<SkImage> &img) {
//        mImgData = data;
//        mDataImg = img;

//        TileSkDrawer *drawer = mTarget->getTexTileDrawer();
//        if(drawer->is)
//    }
//protected:
//    sk_sp<SkImage> mDataImg;
//    uchar *mImgData = nullptr;
//    stdsptr<TileDataGetter> mSrcGetter;
//};

//class TileDataGetter : public GUI_ThreadExecutor {
//public:
//    TileDataGetter(Tile *src, TileDataGetterDependentExecutor *target) {
//        mSrcTile = src;
//    }

//protected:
//    TileDataGetterDependentExecutor *mTarget;
//    Tile *mSrcTile = nullptr;

//    void GUI_process() {
//        SkImageInfo info = SkImageInfo::Make(TILE_DIM,
//                                             TILE_DIM,
//                                             kBGRA_8888_SkColorType,
//                                             kPremul_SkAlphaType,
//                                             nullptr);
//        SkBitmap bitmap;
//        bitmap.allocPixels(info);
//        sk_sp<SkImage> dataImg = SkImage::MakeFromBitmap(bitmap);

//        SkPixmap pix;
//        dataImg->peekPixels(&pix);
//        data = (uchar*)pix.writable_addr();
//        std::memcpy(data, mSrcTile->getData(),
//                    TILE_DIM*TILE_DIM*4*sizeof(uchar));
//        mTarget->dataReady(data, dataImg);
//    }
//};

//class TileDuplicateExecutor : public GUI_ThreadExecutor {
//public:
//    TileDuplicateExecutor(Tile *target, Tile *src) {
//        mSrcGetter = SPtrCreate(TileDataGetter)(src);
//        mTarget = target;
//    }

//protected:
//    Tile *mTarget;
//};
class TilesData;
class Tile {
public:
    Tile(const ushort &x_t, const ushort &y_t,
         TilesData *parentTilesData,
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

    void scheduleTask();

    void setDabsForDrawer();

    void drawSk(SkCanvas *canvas, SkPaint *paint = nullptr);

    void saveToTmp();
    void clearTmp();
    bool writeTile(QIODevice *target);
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
                     Tile *srcTile);

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

        copyDataToDrawer();
    }
    void copyDataToDrawer();
    void setPixel(const int &x, const int &y,
                  const uchar &r, const uchar &g,
                  const uchar &b, const uchar &a);

    void startTransform();

    void finishTransform();
private:
    bool mFinishTransformAfterUpdate = false;
    TilesData *mParentTilesData = nullptr;
    bool mTransformStarted = false;
//    uchar *mSavedData = nullptr;
//    SkBitmap mSavedDataImage;

    bool mPaintInOtherThread;

    stdsptr<TileSkDrawer> mDrawer;
    SkBitmap mDataTileImage;
    SkBitmap mTmpDataTileImage;
    uchar *mDataTmp = nullptr;
    uchar *mData = nullptr;

    QList<Dab> mDabsToPaint;

    void setPosInSurface(const ushort &x_t, const ushort &y_t);

    ushort mPosX = 0;
    ushort mPosY = 0;

    ushort mMaxPaintY = TILE_DIM;
    ushort mMaxPaintX = TILE_DIM;
};

#endif // TILE_H
