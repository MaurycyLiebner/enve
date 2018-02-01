#include "tile.h"
#include <GLES3/gl3.h>
#include <math.h>
#include <stdio.h>
#include "Colors/helpers.h"
#include "tilesdata.h"
#include "mainwindow.h"

void processPaintDabs(const QList<Dab> &dabs,
                      const ushort &maxPaintX,
                      const ushort &maxPaintY,
                      const ushort &dataWidth,
                      uchar *data) {

    foreach(const Dab &dab_t, dabs) {
        qreal cs = cos(dab_t.beta_deg*2*PI/360);
        qreal sn = sin(dab_t.beta_deg*2*PI/360);


        int x_min = floor(dab_t.cx - dab_t.r);
        if(x_min > maxPaintX) {
            continue;
        } else if(x_min < 0) {
            x_min = 0;
        }
        int x_max = ceil(dab_t.cx + dab_t.r);
        if(x_max > maxPaintX) {
            x_max = maxPaintX;
        } else if(x_max < 0) {
            continue;
        }
        int y_min = floor(dab_t.cy - dab_t.r);
        if(y_min > maxPaintY) {
            continue;
        } else if(y_min < 0) {
            y_min = 0;
        }
        int y_max = ceil(dab_t.cy + dab_t.r);
        if(y_max > maxPaintY) {
            y_max = maxPaintY;
        } else if(y_max < 0) {
            continue;
        }

        #pragma omp parallel for
        for(int i = x_min; i < x_max; i++) {
            for(int j = y_min; j < y_max; j++) {
                GLuint id_t = ( j*dataWidth + i)*4;
                qreal dx = i - dab_t.cx;
                qreal dy = j - dab_t.cy;
                qreal dyr = (dy*cs - dx*sn)*dab_t.aspect_ratio;
                qreal dxr = (dy*sn + dx*cs);
                qreal curr_r_frac = (dyr*dyr + dxr*dxr) / (dab_t.r*dab_t.r);
                qreal h_opa;
                if(curr_r_frac > 1) {
                    h_opa = 0;
                } else if(curr_r_frac < dab_t.hardness) {
                    h_opa = curr_r_frac + 1 - curr_r_frac/dab_t.hardness;
                } else {
                    h_opa = dab_t.hardness/(1 - dab_t.hardness)*(1 - curr_r_frac);
                }
                qreal curr_alpha = data[id_t + 3];
                qreal paint_alpha = dab_t.opacity * h_opa * UCHAR_MAX;
                if(dab_t.erase) {
                    qreal alpha_sum = curr_alpha - paint_alpha;

                    if(alpha_sum < 0) {
                        data[id_t + 3] = 0;
                    } else {
                        data[id_t + 3] = alpha_sum;
                    }
                } else {
                    // blue
                    data[id_t] = qMin(255, qMax(0, (int)
                            ((dab_t.blue*paint_alpha*curr_alpha +
                              dab_t.blue*paint_alpha*(UCHAR_MAX - curr_alpha) +
                             data[id_t]*(UCHAR_MAX - paint_alpha))/UCHAR_MAX) ));
                    // green
                    data[id_t + 1] = qMin(255, qMax(0, (int)
                            ((dab_t.green*paint_alpha*curr_alpha +
                              dab_t.green*paint_alpha*(UCHAR_MAX - curr_alpha) +
                             data[id_t + 1]*(UCHAR_MAX - paint_alpha))/UCHAR_MAX) ));
                    // red
                    data[id_t + 2] = qMin(255, qMax(0, (int)
                            ((dab_t.red*paint_alpha*curr_alpha +
                              dab_t.red*paint_alpha*(UCHAR_MAX - curr_alpha) +
                             data[id_t + 2]*(UCHAR_MAX - paint_alpha))/UCHAR_MAX) ));
                    // alpha
                    data[id_t + 3] = qMin(255, qMax(0, (int)
                            ((curr_alpha*paint_alpha +
                              curr_alpha*(UCHAR_MAX - paint_alpha) +
                              paint_alpha*(UCHAR_MAX - curr_alpha))/UCHAR_MAX) ));
                }
            }
        }
    }
}

void Tile::setPixel(const int &x, const int &y,
                    const uchar &r, const uchar &g,
                    const uchar &b, const uchar &a) {
    int blueId = (x + y*TILE_DIM)*4;
    if(mPaintInOtherThread) {
        if(mDrawer->finished()) {
            mData[blueId] = b;
            mData[blueId + 1] = g;
            mData[blueId + 2] = r;
            mData[blueId + 3] = a;
        }
    } else {
        mData[blueId] = b;
        mData[blueId + 1] = g;
        mData[blueId + 2] = r;
        mData[blueId + 3] = a;
    }
}

void Tile::startTransform() {
    if(mTransformStarted) return;
    mParentTilesData->addChangedTile(this);
    mTransformStarted = true;
    std::memcpy(mSavedData, mData, TILE_DIM*TILE_DIM*4*sizeof(uchar));
}

void Tile::finishTransform() {
    if(!mTransformStarted) return;
    if(mPaintInOtherThread) {
        if(mDrawer->finished()) {
            MainWindow::addUndoRedo(new TileChangedUndoRedo(mSavedDataImage,
                                                            mDataTileImage,
                                                            this));
        } else {
            mFinishTransformAfterUpdate = true;
            return;
        }
    }
    mTransformStarted = false;
}

void Tile::duplicateFrom(Tile *tile) {
    if(mPaintInOtherThread) {
        if(mDrawer->finished() && tile->getTexTileDrawer()->finished()) {
            uchar *dataT = tile->getTexTileDrawer()->_data;
            uchar *drawerData = mDrawer->_data;
            std::memcpy(drawerData, dataT, TILE_DIM*TILE_DIM*4*sizeof(uchar));
            std::memcpy(mData, dataT, TILE_DIM*TILE_DIM*4*sizeof(uchar));
        }
    } else {
        uchar *dataT = tile->getData();
        std::memcpy(mData, dataT, TILE_DIM*TILE_DIM*4*sizeof(uchar));
    }
}

void Tile::initializeEmptyTileData() {
    initializeTileData();
    clear();
    if(!mPaintInOtherThread) {
        clearTmp();
    }
}

void Tile::clear() {
    if(mPaintInOtherThread) {
        if(mDrawer->finished()) {
            mDrawer->clearImg();
            updateTexFromDataArray();
        }/* else {
            TileClearExecutor *executor = new TileClearExecutor(this);
            mDrawer->addDependent(executor);
            mDrawer
        }*/
    } else {
        std::memset(mData, 0, TILE_DIM*TILE_DIM*4*sizeof(uchar));
    }
}

void Tile::setTileWidth(const ushort &width_t) {
    if(width_t == mMaxPaintX) return;
    mMaxPaintX = width_t;
    if(mDrawer != NULL) {
        mDrawer->maxPaintX = mMaxPaintX;
    }
    addScheduler();
}

void Tile::setTileHeight(const ushort &height_t) {
    if(height_t == mMaxPaintY) return;
    mMaxPaintY = height_t;
    if(mDrawer != NULL) {
        mDrawer->maxPaintY = mMaxPaintY;
    }
    addScheduler();
}

void Tile::resetTileSize() {
    setTileHeight(TILE_DIM);
    setTileWidth(TILE_DIM);
}

TileSkDrawer *Tile::getTexTileDrawer() {
    return mDrawer.get();
}

void Tile::addScheduler() {
    if(mPaintInOtherThread) {
        startTransform();
        mParentTilesData->incNumberTilesBeingProcessed();
        mDrawer->addScheduler();
    } else {
        processPaintDabs(mDabsToPaint,
                         mMaxPaintX, mMaxPaintY, TILE_DIM,
                         (uchar*)mDataTileImage.getPixels());
        mDabsToPaint.clear();
    }
}

void Tile::setDabsForDrawer() {
    mDrawer->_dabsToPaint = mDabsToPaint;
    mDabsToPaint.clear();
}

void Tile::drawSk(SkCanvas *canvas, SkPaint *paint) {
    canvas->drawBitmap(mTmpDataTileImage, mPosX, mPosY, paint);
    canvas->drawBitmap(mDataTileImage, mPosX, mPosY, paint);
}

void Tile::saveToTmp() {
    mTmpDataTileImage.swap(mDataTileImage);
    uchar *dataT = mData;
    mData = mDataTmp;
    mDataTmp = dataT;
}

void Tile::clearTmp() {
    std::memset(mDataTmp, 0, TILE_DIM*TILE_DIM*4*sizeof(uchar));
}

Tile::Tile(const ushort &x_t, const ushort &y_t,
           TilesData *parentTilesData,
           const bool &paintInOtherThread) {
    mParentTilesData = parentTilesData;
    mPaintInOtherThread = paintInOtherThread;
    setPosInSurface(x_t, y_t);

    resetTileSize();
}

Tile::~Tile() {
    if(mDrawer == NULL) return;
    mDrawer->parentTile = NULL;
}

void Tile::initializeDrawer() {
    if(mPaintInOtherThread) {
        mDrawer = (new TileSkDrawer(this, mPosX, mPosY))->ref<TileSkDrawer>();
        mDrawer->maxPaintX = mMaxPaintX;
        mDrawer->maxPaintY = mMaxPaintY;
    }
}

void Tile::clearTileData() {
    mData = NULL;
    mDataTileImage.reset();
    mDrawer.reset();
}

void Tile::replaceData(const ushort &srcX, const ushort &srcY,
                       const ushort &targetX, const ushort &targetY,
                       const ushort &width, const ushort &height,
                       Tile *srcTile) {
    if(height == 0 || width == 0) return;
    if(mPaintInOtherThread) {
        if(!mDrawer->finished() ||
           !srcTile->getTexTileDrawer()->finished()) {
            return;
        }
    }
    uchar *srcData = srcTile->getData();
    int iFirst = 0;
    int iLast = width - 1;
    int iInc = 1;
    int jFirst = 0;
    int jLast = height - 1;
    int jInc = 1;
    if(srcData == mData) {
        if(targetX > srcX) {
            iInc = -1;
            iFirst = width - 1;
            iLast = 0;
        }
        if(targetY > srcY) {
            jInc = -1;
            jFirst = height - 1;
            jLast = 0;
        }
    }
    int i = iFirst;
    while(true) {
        int j = jFirst;
        while(true) {
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
            if(j == jLast) break;
            j += jInc;
        }
        if(i == iLast) break;
        i += iInc;
    }

    copyDataToDrawer();
}

void Tile::initializeTileData() {
    SkImageInfo info = SkImageInfo::Make(TILE_DIM,
                                         TILE_DIM,
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    mDataTileImage.allocPixels(info);
    mDataTileImage.setIsVolatile(true);

    mData = (uchar*)mDataTileImage.getPixels();

    mSavedDataImage.allocPixels(info);
    mSavedDataImage.setIsVolatile(true);

    mSavedData = (uchar*)mSavedDataImage.getPixels();

    if(mPaintInOtherThread) {
        initializeDrawer();
    } else {
        mTmpDataTileImage.allocPixels(info);
        mTmpDataTileImage.setIsVolatile(true);
        mDataTmp = (uchar*)mTmpDataTileImage.getPixels();
    }
}

void Tile::setPosInSurface(const ushort &x_t, const ushort &y_t) {
    mPosX = x_t;
    mPosY = y_t;
}

void Tile::addDabToDraw(qreal cx,
                        qreal cy,
                        const qreal &hardness,
                        const qreal &opa,
                        const qreal &aspect_ratio,
                        const qreal &r,
                        const qreal &beta_deg,
                        const qreal &red,
                        const qreal &green,
                        const qreal &blue,
                        const bool &erase) {
    cx -= mPosX;
    cy -= mPosY;

    mDabsToPaint.append(
                Dab(cx, cy,
                    hardness, opa,
                    aspect_ratio, r,
                    beta_deg,
                    red, green, blue,
                    erase) );
}

void Tile::getColor(qreal cx,
                    qreal cy,
                    const qreal &r,
                    const qreal &aspect_ratio,
                    const qreal &cs,
                    const qreal &sn,
                    const qreal &hardness,
                    const qreal &opacity,
                    qreal *red_sum, qreal *green_sum,
                    qreal *blue_sum, qreal *alpha_sum,
                    qreal *weight_sum) {
    cx -= mPosX;
    cy -= mPosY;

    int x_min = floor(cx - r);
    if(x_min > mMaxPaintX) {
        return;
    } else if(x_min < 0){
        x_min = 0;
    }
    int x_max = ceil(cx + r);
    if(x_max > mMaxPaintX) {
        x_max = mMaxPaintX;
    } else if(x_max < 0) {
        return;
    }
    int y_min = floor(cy - r);
    if(y_min > mMaxPaintY) {
        return;
    } else if(y_min < 0) {
        y_min = 0;
    }
    int y_max = ceil(cy + r);
    if(y_max > mMaxPaintY) {
        y_max = mMaxPaintY;
    } else if(y_max < 0) {
        return;
    }

    qreal red_sum_t = 0.;
    qreal green_sum_t = 0.;
    qreal blue_sum_t = 0.;
    qreal alpha_sum_t = 0.;
    qreal weight_sum_t = 0.;
    #pragma omp parallel for reduction(+: red_sum_t, green_sum_t, blue_sum_t, alpha_sum_t, weight_sum_t)
    for(short i = x_min; i < x_max; i++) {
        for(short j = y_min; j < y_max; j++) {
            GLuint col_val_id = (j*TILE_DIM + i)*4;
            qreal dx = i - cx;
            qreal dy = j - cy;
            qreal dyr = (dy*cs - dx*sn)*aspect_ratio;
            qreal dxr = (dy*sn + dx*cs);
            qreal curr_r_frac = (dyr*dyr + dxr*dxr) / (r*r);
            qreal h_opa;
            if(curr_r_frac > 1) {
                h_opa = 0;
            } else if(curr_r_frac < hardness) {
                h_opa = curr_r_frac + 1 - curr_r_frac/hardness;
            } else {
                h_opa = hardness/(1 - hardness)*(1 - curr_r_frac);
            }
            qreal alpha_t = mData[col_val_id + 3]/
                    (qreal)UCHAR_MAX;
            qreal weight_t = opacity * h_opa;
            weight_sum_t += weight_t;
            if(alpha_t < 0.01) continue;
            red_sum_t += mData[col_val_id + 2]*weight_t/
                    ((qreal)UCHAR_MAX*alpha_t);
            green_sum_t += mData[col_val_id + 1]*weight_t/
                    ((qreal)UCHAR_MAX*alpha_t);
            blue_sum_t += mData[col_val_id ]*weight_t/
                    ((qreal)UCHAR_MAX*alpha_t);
            alpha_sum_t += weight_t*alpha_t;
        }
    }

    *red_sum = red_sum_t;
    *green_sum = green_sum_t;
    *blue_sum = blue_sum_t;
    *alpha_sum = alpha_sum_t;
    *weight_sum = weight_sum_t;
}

void Tile::updateTexFromDataArray() {
    uchar *dataT = mDrawer->_data;
    std::memcpy(mData, dataT, TILE_DIM*TILE_DIM*4*sizeof(uchar));
    mParentTilesData->decNumberTilesBeingProcessed();
    if(mFinishTransformAfterUpdate) {
        finishTransform();
    }
}

void Tile::copyDataToDrawer() {
    uchar *dataT = mDrawer->_data;
    std::memcpy(dataT, mData, TILE_DIM*TILE_DIM*4*sizeof(uchar));
}

TileSkDrawer::TileSkDrawer(Tile *parentTileT,
                           const ushort &xT,
                           const ushort &yT) {
    parentTile = parentTileT;
    posX = xT;
    posY = yT;

    SkImageInfo info = SkImageInfo::Make(TILE_DIM,
                                         TILE_DIM,
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    tileImg = SkImage::MakeFromBitmap(bitmap);

    SkPixmap pix;
    tileImg->peekPixels(&pix);
    _data = (uchar*)pix.writable_addr();
}

TileSkDrawer::~TileSkDrawer() {
}

void TileSkDrawer::drawSk(SkCanvas *canvas, SkPaint *paint) const {
    canvas->drawImage(tileImg, posX, posY, paint);
}

void TileSkDrawer::schedulerProccessed() {
    _ScheduledExecutor::schedulerProccessed();
    if(parentTile != NULL) {
        parentTile->setDabsForDrawer();
    }
}

void TileSkDrawer::_processUpdate() {
    processPaintDabs(_dabsToPaint,
                     _maxPaintX, _maxPaintY, TILE_DIM,
                     _data);
    _dabsToPaint.clear();
}

void TileSkDrawer::afterUpdate() {
    if(parentTile != NULL) {
        parentTile->updateTexFromDataArray();
    }
    _ScheduledExecutor::afterUpdate();
}

void TileSkDrawer::clearImg() {
    std::memset(_data, 0, TILE_DIM*TILE_DIM*4*sizeof(uchar));
}
