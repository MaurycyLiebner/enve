#include "tile.h"
#include <GLES3/gl3.h>
#include <math.h>
#include <stdio.h>
#include "Colors/helpers.h"

void Tile::clear() {
    mDrawer->clearImg();
    updateTexFromDataArray();
    mDrawer->addScheduler();
}

void Tile::setTileWidth(const ushort &width_t) {
    mMaxPaintX = width_t;
    mDrawer->maxPaintX = mMaxPaintX;
}

void Tile::setTileHeight(const ushort &height_t) {
    mMinPaintY = TILE_DIM - height_t;
    mDrawer->minPaintY = mMinPaintY;
}

void Tile::resetTileSize() {
    setTileHeight(TILE_DIM);
    setTileWidth(TILE_DIM);
}

TileSkDrawer *Tile::getTexTileDrawer() {
    return mDrawer.get();
}

void Tile::setDabsForDrawer() {
    mDrawer->dabsToPaint = mDabsToPaint;
    mDabsToPaint.clear();
}

Tile::Tile(const ushort &x_t, const ushort &y_t) {
    setPosInSurface(x_t, y_t);
    mDrawer = (new TileSkDrawer(this, x_t, y_t))->ref<TileSkDrawer>();
    resetTileSize();

    SkImageInfo info = SkImageInfo::Make(TILE_DIM,
                                         TILE_DIM,
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    mDataTileImage = SkImage::MakeFromBitmap(bitmap);
    SkPixmap pix;
    mDataTileImage->peekPixels(&pix);
    mData = (uchar*)pix.writable_addr();

    clear();
}

void Tile::setPosInSurface(const ushort &x_t, const ushort &y_t) {
    mPosX = x_t;
    mPosY = y_t;
}

void Tile::partOfSurfRectInTile(const short &surf_x,
                                const short &surf_y,
                                const short &width,
                                const short &height,
                                short *rect_x, short *rect_y,
                                short *width_t, short *height_t) {
    if(surf_x > mPosX) {
        *rect_x = surf_x - mPosX;
        *width_t = width;
        if(*rect_x + width > TILE_DIM) {
            *width_t = TILE_DIM - *rect_x;
        } else {
            *width_t = width;
        }
    } else {
        *rect_x = 0;
        *width_t = width - (mPosX - surf_x);
        if(*width_t > TILE_DIM) {
            *width_t = TILE_DIM;
        }
    }
    if(surf_y > mPosY) {
        *rect_y = surf_y - mPosY;
        *height_t = height;
        if(*rect_y + height > TILE_DIM) {
            *height_t = TILE_DIM - *rect_y;
        } else {
            *width_t = width;
        }
    } else {
        *rect_y = 0;
        *height_t = height - (mPosY - surf_y);
        if(*height_t > TILE_DIM) {
            *height_t = TILE_DIM;
        }
    }
}

bool Tile::surfRectInTile(const int &surf_x, const int &surf_y,
                          const int &width, const int &height) {
    if(surf_x > mPosX + TILE_DIM) {
        return false;
    }
    if(surf_y > mPosY + TILE_DIM) {
        return false;
    }
    if(surf_x + width < mPosX) {
        return false;
    }
    if(surf_y + height < mPosY) {
        return false;
    }
    return true;
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
    cy = TILE_DIM - cy;

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
        x_max = 0;
    }
    int y_min = floor(cy - r);
    if(y_min > TILE_DIM) {
        return;
    } else if(y_min < mMinPaintY) {
        y_min = mMinPaintY;
    }
    int y_max = ceil(cy + r);
    if(y_max > TILE_DIM) {
        y_max = TILE_DIM;
    } else if(y_max < mMinPaintY) {
        y_max = mMinPaintY;
    }

    qreal red_sum_t = 0.f;
    qreal green_sum_t = 0.f;
    qreal blue_sum_t = 0.f;
    qreal alpha_sum_t = 0.f;
    qreal weight_sum_t = 0.f;
    //#pragma omp parallel for reduction(+: red_sum_t, green_sum_t, blue_sum_t, alpha_sum_t, weight_sum_t)
    for(short i = x_min; i < x_max; i++) {
        for(short j = y_min; j < y_max; j++) {
            GLuint col_val_id = ( (TILE_DIM - j - 1 + mMinPaintY)*TILE_DIM + i)*4;
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
            qreal alpha_t = mData[col_val_id + 3]/(float)UCHAR_MAX;
            qreal weight_t = opacity * h_opa;
            weight_sum_t += weight_t;
            qreal alpha_sum_inc_t = weight_t*alpha_t;
            red_sum_t += mData[col_val_id]/(float)UCHAR_MAX*alpha_sum_inc_t;
            green_sum_t += mData[col_val_id + 1]/(float)UCHAR_MAX*alpha_sum_inc_t;
            blue_sum_t += mData[col_val_id + 2]/(float)UCHAR_MAX*alpha_sum_inc_t;
            alpha_sum_t += alpha_sum_inc_t;
        }
    }

    *red_sum += red_sum_t;
    *green_sum += green_sum_t;
    *blue_sum += blue_sum_t;
    *alpha_sum += alpha_sum_t;
    *weight_sum += weight_sum_t;
}
void Tile::updateTexFromDataArray() {
    uchar *dataT = mDrawer->data;
    for(int i = 0; i < TILE_DIM*TILE_DIM*4; i++) {
        mData[i] = dataT[i];
    }
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
    data = (uchar*)pix.writable_addr();
}

void TileSkDrawer::drawSk(SkCanvas *canvas, SkPaint *paint) const {
    canvas->drawImage(tileImg, posX, posY, paint);
}

void TileSkDrawer::beforeUpdate() {
    Updatable::beforeUpdate();
    parentTile->setDabsForDrawer();
}

void TileSkDrawer::processUpdate() {
    foreach(const Dab &dab_t, dabsToPaint) {
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
            x_max = 0;
        }
        int y_min = floor(dab_t.cy - dab_t.r);
        if(y_min > TILE_DIM) {
            continue;
        } else if(y_min < minPaintY) {
            y_min = minPaintY;
        }
        int y_max = ceil(dab_t.cy + dab_t.r);
        if(y_max > TILE_DIM) {
            y_max = TILE_DIM;
        } else if(y_max < minPaintY) {
            y_max = minPaintY;
        }

        //#pragma omp parallel for
        for(int i = x_min; i < x_max; i++) {
            for(int j = y_min; j < y_max; j++) {
                GLuint id_t = ( j*TILE_DIM + i)*4;
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
                    qreal alpha_sum = curr_alpha + paint_alpha;
                    // red
                    data[id_t] =
                            (dab_t.red*UCHAR_MAX*paint_alpha +
                             data[id_t]*curr_alpha)/alpha_sum;
                    // green
                    data[id_t + 1] =
                            (dab_t.green*UCHAR_MAX*paint_alpha +
                             data[id_t + 1]*curr_alpha)/alpha_sum;
                    // blue
                    data[id_t + 2] =
                            (dab_t.blue*UCHAR_MAX*paint_alpha +
                             data[id_t + 2]*curr_alpha)/alpha_sum;
                    // alpha
                    if(alpha_sum > UCHAR_MAX) {
                        data[id_t + 3] = UCHAR_MAX;
                    } else {
                        data[id_t + 3] = alpha_sum;
                    }
                }
            }
        }
    }

    dabsToPaint.clear();
}

void TileSkDrawer::afterUpdate() {
    Updatable::afterUpdate();
    parentTile->updateTexFromDataArray();
}

void TileSkDrawer::clearImg() {
    for(int i = 0; i < TILE_DIM*TILE_DIM*4; i++) {
        data[i] = 125;
    }
}
