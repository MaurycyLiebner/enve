#include "surface.h"
#include <math.h>
#include <algorithm>
#include "GL/gl.h"
#include "Colors/helpers.h"

Surface::Surface(const ushort &width_t,
                 const ushort &height_t,
                 const qreal &scale,
                 const bool &paintOnOtherThread) {
    mScale = scale;
    mPaintInOtherThread = paintOnOtherThread;
    setSize(width_t, height_t);
}

qreal Surface::countDabsTo(const qreal &dist_between_dabs,
                           const qreal &x, const qreal &y) {
    qreal dx = x - mLastPaintedStrokeX;
    qreal dy = y - mLastPaintedStrokeY;

    return sqrt(dx*dx + dy*dy)/dist_between_dabs;
}

void replaceIfSmaller(const short &potentially_smaller,
                      short *to_be_replaced) {
    if(potentially_smaller < *to_be_replaced) {
        *to_be_replaced = potentially_smaller;
    }
}

void replaceIfBigger(const short &potentially_bigger,
                     short *to_be_replaced) {
    if(potentially_bigger > *to_be_replaced) {
        *to_be_replaced = potentially_bigger;
    }
}

void Surface::getTileIdsOnRect(const qreal &x_min,
                               const qreal &x_max,
                               const qreal &y_min,
                               const qreal &y_max,
                               short *tile_x_min,
                               short *tile_x_max,
                               short *tile_y_min,
                               short *tile_y_max) {
    *tile_x_min = floor(x_min/TILE_DIM);
    if(*tile_x_min < 0) {
        *tile_x_min = 0;
    }
    *tile_x_max = ceil(x_max/TILE_DIM);
    if(*tile_x_max >= mNTileCols) {
        *tile_x_max = mNTileCols - 1;
    }
    *tile_y_min = floor(y_min/TILE_DIM);
    if(*tile_y_min < 0) {
        *tile_y_min = 0;
    }
    *tile_y_max = ceil(y_max/TILE_DIM);
    if(*tile_y_max >= mNTileRows) {
        *tile_y_max = mNTileRows - 1;
    }
}

void Surface::getColor(const qreal &cx,
                       const qreal &cy,
                       const qreal &hardness,
                       const qreal &opa,
                       const qreal &aspect_ratio,
                       const qreal &r,
                       const qreal &beta_deg,
                       qreal *red, qreal *green,
                       qreal *blue, qreal *alpha) {
    qreal cs = cos(beta_deg*2*PI/360);
    qreal sn = sin(beta_deg*2*PI/360);

    qreal red_sum = 0.;
    qreal green_sum = 0.;
    qreal blue_sum = 0.;
    qreal alpha_sum = 0.;
    qreal weight_sum = 0.;
    short min_tile_x;
    short max_tile_x;
    short min_tile_y;
    short max_tile_y;
    getTileIdsOnRect(cx - r, cx + r, cy - r, cy + r,
                     &min_tile_x, &max_tile_x, &min_tile_y, &max_tile_y);
    for(short i = min_tile_x; i <= max_tile_x; i++) {
        for(short j = min_tile_y; j <= max_tile_y; j++) {
            Tile *tile_t = mTiles[j][i];
            tile_t->getColor(cx, cy,
                             r, aspect_ratio, cs, sn,
                             hardness, opa,
                             &red_sum, &green_sum,
                             &blue_sum, &alpha_sum,
                             &weight_sum);
        }
    }
    if(alpha_sum < 0.01) {
        *red = 0.;
        *green = 0.;
        *blue = 0.;
        *alpha = 0.;
    } else {
        *red = red_sum/weight_sum;
        *green = green_sum/weight_sum;
        *blue = blue_sum/weight_sum;
        *alpha = alpha_sum/weight_sum;
    }
}

void Surface::clear() {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->clear();
        }
    }
}

void Surface::getTileDrawers(QList<TileSkDrawer*> *tileDrawers) {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            tileDrawers->append(mTiles[j][i]->getTexTileDrawer());
        }
    }
}

void Surface::clearTmp() {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->clearTmp();
        }
    }
}

void Surface::saveToTmp() {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->saveToTmp();
        }
    }
}

void Surface::strokeTo(Brush *brush,
                       qreal x, qreal y,
                       const qreal &pressure,
                       const ushort &dt,
                       const bool &erase) {
    x *= mScale;
    y *= mScale;
    qreal dist_between_dabs =
            brush->getDistBetweenDabsPx()*mScale;
    qreal stroke_dx = x - mLastEventStrokeX;
    qreal stroke_dy = y - mLastEventStrokeY;
    mLastEventStrokeX = x;
    mLastEventStrokeY = y;
    qreal stroke_vx = stroke_dx*50/dt;
    qreal stroke_vy = stroke_dy*50/dt;
    qreal stroke_vel = mLastEventStrokeVel*0.8 +
            0.2*sqrt(stroke_vx*stroke_vx +
                     stroke_vy*stroke_vy);
    mLastEventStrokeVel = stroke_vel;

    applyXYNoise(brush->getStrokePositionNoisePx()*mScale,
                 &mPreviousStrokeXNoise,
                 &mNextStrokeXNoise,
                 &mPreviousStrokeYNoise,
                 &mNextStrokeYNoise,
                 brush->getStrokePositionNoiseFrequency(),
                 &mStrokeNoiseCount, &x, &y);

    qreal dabs_to = countDabsTo(dist_between_dabs, x, y);

    qreal first_dab_dx = x - mLastPaintedStrokeX;
    qreal first_dab_dy = y - mLastPaintedStrokeY;;
    qreal dabs_dx = first_dab_dx;
    qreal dabs_dy = first_dab_dy;

    if(isZero(first_dab_dx) && isZero(first_dab_dy)) {
        first_dab_dx = 0.;
        first_dab_dy = 0.;
        dabs_dx = 0.;
        dabs_dy = 0.;
    } else {
        normalize(&first_dab_dx, &first_dab_dy,
                  dist_between_dabs);
        normalize(&dabs_dx, &dabs_dy,
                  dist_between_dabs);
    }
    qreal first_dab_x = mLastPaintedStrokeX + first_dab_dx;
    qreal first_dab_y = mLastPaintedStrokeY + first_dab_dy;

    short min_affected_tile_x = 10000;
    short max_affected_tile_x = 0;
    short min_affected_tile_y = 10000;
    short max_affected_tile_y = 0;

    short dabs_to_i = floor(dabs_to);
    if(dabs_to_i <= 0) return;
    qreal rotation_delay = brush->getRotationDelay();
    qreal angle_t =
            getAngleDeg(x - mLastPaintedStrokeX,
                        y - mLastPaintedStrokeY, 0., -1.);
    if(angle_t > 180.) {
        angle_t -= 180.;
    } else if(angle_t < 0.) {
        angle_t += 180.;
    }
    qreal angle_diff = fabs(angle_t - mLastStrokeBeta);
    qreal dest_angle;
    if(angle_diff > 120.) {
        dest_angle = angle_t;
    } else {
        dest_angle = angle_t*(1 - rotation_delay)*
                brush->getRotationInfluence() +
                mLastStrokeBeta*rotation_delay;
    }

    if(dest_angle > 180.) {
        dest_angle -= 180.;
    } else if(dest_angle < 0.) {
        dest_angle += 180.;
    }
    mLastStrokeBeta = dest_angle;
    qreal dab_r =
       qMin(brush->getRadius()*5,
            brush->getRadius() +
            brush->getPressureRadiusGainPx()*pressure +
            brush->getSpeedRadiusGain()*stroke_vel)*mScale;
    qreal dab_hardness = brush->getHardness() +
            brush->getPressureHardnessGain()*pressure +
            brush->getSpeedHardnessGain()*stroke_vel;
    qreal dab_opa = brush->getOpacity() +
            brush->getPressureOpacityGain()*pressure +
            brush->getSpeedOpacityGain()*stroke_vel;
    qreal dab_aspect_ratio = brush->getAspectRatio() +
            brush->getPressureAspectRatioGain()*pressure;

    qreal smudge_red;
    qreal smudge_green;
    qreal smudge_blue;
    qreal smudge_alpha;
    getColor(mLastPaintedStrokeX, mLastPaintedStrokeY,
             dab_hardness, dab_opa,
             dab_aspect_ratio, dab_r, dest_angle,
             &smudge_red, &smudge_green,
             &smudge_blue, &smudge_alpha);
    brush->addPickedUpRGBAFromNewStroke(smudge_red,
                                        smudge_green,
                                        smudge_blue,
                                        smudge_alpha);
    brush->getPickedUpRGBA(&smudge_red,
                           &smudge_green,
                           &smudge_blue,
                           &smudge_alpha);


    qreal stroke_red;
    qreal stroke_green;
    qreal stroke_blue;
    qreal dab_alpha;
    bool fixed_color = isZero(brush->getHueNoise()) &&
            isZero(brush->getSaturationNoise()) &&
            isZero(brush->getValueNoise());

    brush->getRGBA(&stroke_red,
                   &stroke_green,
                   &stroke_blue,
                   &dab_alpha);
    dab_alpha += brush->getPressureAlphaGain()*pressure;

    qreal alpha_sum_t = dab_alpha + smudge_alpha;
    stroke_red = (stroke_red*dab_alpha +
                  smudge_alpha*smudge_red)/alpha_sum_t;
    stroke_green = (stroke_green*dab_alpha +
                    smudge_alpha*smudge_green)/alpha_sum_t;
    stroke_blue = (stroke_blue*dab_alpha +
                   smudge_alpha*smudge_blue)/alpha_sum_t;
    if(alpha_sum_t > 1.) {
        alpha_sum_t = 1.;
    }

    qreal dab_red = stroke_red;
    qreal dab_green = stroke_green;
    qreal dab_blue = stroke_blue;

    qreal stroke_h;
    qreal stroke_s;
    qreal stroke_v;
    if(!fixed_color) {
        stroke_h = stroke_red;
        stroke_s = stroke_green;
        stroke_v = stroke_blue;
        qrgb_to_hsv(&stroke_h, &stroke_s, &stroke_v);
    }

    for(short i = 0; i < dabs_to_i; i++) {
        if(!fixed_color) {
            dab_red = stroke_h;
            applyNoise(brush->getHueNoise(),
                       &mPreviousHueNoise,
                       &mNextHueNoise,
                       brush->getHueNoiseFrequency(),
                       &mHueNoiseCount, &dab_red);
            dab_green = stroke_s;
            applyNoise(brush->getSaturationNoise(),
                       &mPreviousSaturationNoise,
                       &mNextSaturationNoise,
                       brush->getSaturationNoiseFrequency(),
                       &mSaturationNoiseCount, &dab_green);
            dab_blue = stroke_v;
            applyNoise(brush->getValueNoise(),
                       &mPreviousValueNoise,
                       &mNextValueNoise,
                       brush->getValueNoiseFrequency(),
                       &mValueNoiseCount, &dab_blue);
            qhsv_to_rgb(&dab_red, &dab_green, &dab_blue);
        }
        qreal dab_x = first_dab_x + i*dabs_dx +
                getNoise(brush->getDabPositionNoisePx()*mScale);
        qreal dab_y = first_dab_y + i*dabs_dy +
                getNoise(brush->getDabPositionNoisePx()*mScale);
        mLastPaintedStrokeX = dab_x;
        mLastPaintedStrokeY = dab_y;
        mLastDabRotationInc += brush->getRotationBetweenDabs();
        if(mLastDabRotationInc > 180.f) {
            mLastDabRotationInc -= 180.f;
        }

        qreal dab_x_min = dab_x - dab_r;
        qreal dab_x_max = dab_x + dab_r;
        qreal dab_y_min = dab_y - dab_r;
        qreal dab_y_max = dab_y + dab_r;

        short dab_min_tile_x;
        short dab_max_tile_x;
        short dab_min_tile_y;
        short dab_max_tile_y;
        getTileIdsOnRect(dab_x_min, dab_x_max,
                         dab_y_min, dab_y_max,
                         &dab_min_tile_x, &dab_max_tile_x,
                         &dab_min_tile_y, &dab_max_tile_y);

        replaceIfSmaller(dab_min_tile_x, &min_affected_tile_x);
        replaceIfSmaller(dab_min_tile_y, &min_affected_tile_y);
        replaceIfBigger(dab_max_tile_x, &max_affected_tile_x);
        replaceIfBigger(dab_max_tile_y, &max_affected_tile_y);
        qreal dab_rot = dest_angle + brush->getInitialRotation() +
                mLastDabRotationInc +
                getNoise(brush->getRotationNoise() );
        //#pragma omp parallel for
        for(short tx = dab_min_tile_x; tx <= dab_max_tile_x; tx++) {
            for(short ty = dab_min_tile_y; ty <= dab_max_tile_y; ty++) {
                mTiles[ty][tx]->addDabToDraw(dab_x, dab_y,
                                            dab_hardness,
                                            dab_opa*alpha_sum_t,
                                            dab_aspect_ratio,
                                            dab_r, dab_rot,
                                            dab_red, dab_green,
                                            dab_blue, erase);
            }
        }
    }

    for(short tx = min_affected_tile_x; tx <= max_affected_tile_x; tx++) {
        for(short ty = min_affected_tile_y; ty <= max_affected_tile_y; ty++) {
            mTiles[ty][tx]->addScheduler();
        }
    }

    mLastStrokePress = pressure;
}

void Surface::startNewStroke(Brush *brush,
                             qreal x, qreal y,
                             const qreal &pressure) {
    x *= mScale;
    y *= mScale;
    mStrokeNoiseCount = UCHAR_MAX;
    mPreviousStrokeXNoise = 0.;
    mPreviousStrokeYNoise = 0.;
    mNextStrokeXNoise = 0.;
    mNextStrokeYNoise = 0.;

    mRotationNoiseCount = UCHAR_MAX;
    mPreviousRotationNoise = 0.;
    mNextRotationNoise = 0.;

    mHueNoiseCount = UCHAR_MAX;
    mPreviousHueNoise = 0.;
    mNextHueNoise = 0.;

    mSaturationNoiseCount = UCHAR_MAX;
    mPreviousSaturationNoise = 0.;
    mNextSaturationNoise = 0.;

    mValueNoiseCount = UCHAR_MAX;
    mPreviousValueNoise = 0.;
    mNextValueNoise = 0.;

    mLastEventStrokeX = x;
    mLastEventStrokeY = y;
    mLastPaintedStrokeX = x;
    mLastPaintedStrokeY = y;
    mLastXSpeedOffset = 0.;
    mLastYSpeedOffset = 0.;
    mLastStrokePress = pressure;
    brush->resetPickedUpRGBA();

    mNextSecondColorAlpha = 0.;
    mPreviousSecondColorAlpha = 0.;
    mSecondColorAlphaCount = UCHAR_MAX;
}

Tile *Surface::getTile(const ushort &tile_col,
                       const ushort &tile_row) {
    return mTiles[tile_row][tile_col];
}

void Surface::setSize(const ushort &width_t,
                      const ushort &height_t) {
    // initialize tiles
    ushort n_tile_cols_t = ceil(width_t/(qreal)TILE_DIM);
    ushort n_tile_rows_t = ceil(height_t/(qreal)TILE_DIM);

    Tile ***tiles_t = new Tile**[n_tile_rows_t];

    for(ushort rw = 0; rw < n_tile_rows_t; rw++) {
        tiles_t[rw] = new Tile*[n_tile_cols_t];
        ushort first_new_col_in_row = 0;
        if(rw < mNTileRows) {
            first_new_col_in_row = mNTileCols;
            for(ushort cl = 0; cl < mNTileCols; cl++) {
                Tile *tile_t = mTiles[rw][cl];
                if(cl < n_tile_cols_t) {
                    tile_t->resetTileSize();
                    tiles_t[rw][cl] = tile_t;
                } else {
                    delete tile_t;
                }
            }
            delete[] mTiles[rw];
        }

        for(ushort cl = first_new_col_in_row; cl < n_tile_cols_t; cl++) {
            tiles_t[rw][cl] = new Tile(cl*TILE_DIM, rw*TILE_DIM,
                                       mPaintInOtherThread);
        }

    }
    if(mTiles != NULL) {
        delete[] mTiles;
    }

    mTiles = tiles_t;
    mWidth = width_t;
    mHeight = height_t;
    mNTileRows = n_tile_rows_t;
    mNTileCols = n_tile_cols_t;


    ushort last_row_height = mHeight%TILE_DIM;
    if(last_row_height != 0) {
        for(int i = 0; i < mNTileCols; i++) {
            mTiles[mNTileRows - 1][i]->setTileHeight(last_row_height);
        }
    }
    ushort last_column_width = mWidth%TILE_DIM;
    if(last_column_width != 0) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][mNTileCols - 1]->setTileWidth(last_column_width);
        }
    }

}
