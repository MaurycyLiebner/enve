#include "surface.h"
#include <math.h>
#include <algorithm>
#include "GL/gl.h"
#include "Colors/helpers.h"

Surface::Surface(ushort width_t, ushort height_t)
{
    setSize(width_t, height_t);
}

void Surface::paintGL()
{
    glEnable( GL_TEXTURE_2D );
    glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for(ushort rw = 0; rw < n_tile_rows; rw++)
    {
        for(ushort cl = 0; cl < n_tile_cols; cl++)
        {
            tiles[rw][cl]->paintGL();
        }
    }

    glDisable(GL_BLEND);
    glDisable( GL_TEXTURE_2D);
}

qreal Surface::countDabsTo(qreal dist_between_dabs, qreal x, qreal y)
{
    qreal dx = x - last_painted_stroke_x;
    qreal dy = y - last_painted_stroke_y;

    return sqrt(dx*dx + dy*dy)/dist_between_dabs;
}

void replaceIfSmaller(short potentially_smaller, short *to_be_replaced)
{
    if(potentially_smaller < *to_be_replaced)
    {
        *to_be_replaced = potentially_smaller;
    }
}

void replaceIfBigger(short potentially_bigger, short *to_be_replaced)
{
    if(potentially_bigger > *to_be_replaced)
    {
        *to_be_replaced = potentially_bigger;
    }
}

void Surface::getTileIdsOnRect(qreal x_min, qreal x_max, qreal y_min, qreal y_max,
                               short *tile_x_min, short *tile_x_max, short *tile_y_min, short *tile_y_max)
{
    *tile_x_min = floor(x_min/TILE_DIM);
    if(*tile_x_min < 0)
    {
        *tile_x_min = 0;
    }
    *tile_x_max = ceil(x_max/TILE_DIM);
    if(*tile_x_max >= n_tile_cols)
    {
        *tile_x_max = n_tile_cols - 1;
    }
    *tile_y_min = floor(y_min/TILE_DIM);
    if(*tile_y_min < 0)
    {
        *tile_y_min = 0;
    }
    *tile_y_max = ceil(y_max/TILE_DIM);
    if(*tile_y_max >= n_tile_rows)
    {
        *tile_y_max = n_tile_rows - 1;
    }
}

void Surface::getColor(qreal cx, qreal cy,
                    qreal hardness, qreal opa, qreal aspect_ratio, qreal r, qreal beta_deg,
                    qreal *red, qreal *green, qreal *blue, qreal *alpha)
{
    qreal cs = cos(beta_deg*2*PI/360);
    qreal sn = sin(beta_deg*2*PI/360);

    qreal red_sum = 0.f;
    qreal green_sum = 0.f;
    qreal blue_sum = 0.f;
    qreal alpha_sum = 0.f;
    qreal weight_sum = 0.f;
    short min_tile_x;
    short max_tile_x;
    short min_tile_y;
    short max_tile_y;
    getTileIdsOnRect(cx - r, cx + r, cy - r, cy + r,
                     &min_tile_x, &max_tile_x, &min_tile_y, &max_tile_y);
    for(short i = min_tile_x; i <= max_tile_x; i++)
    {
        for(short j = min_tile_y; j <= max_tile_y; j++)
        {
            Tile *tile_t = tiles[j][i];
            tile_t->getColor(cx, cy,
                             r, aspect_ratio, cs, sn,
                             hardness, opa,
                             &red_sum, &green_sum, &blue_sum, &alpha_sum, &weight_sum);
        }
    }
    if(alpha_sum < 0.0001f)
    {
        *red = 0.f;
        *green = 0.f;
        *blue = 0.f;
        *alpha = 0.f;
    }
    else
    {
        *red = red_sum/alpha_sum;
        *green = green_sum/alpha_sum;
        *blue = blue_sum/alpha_sum;
        *alpha = alpha_sum/weight_sum;
    }
}

void Surface::savePixelsToPngArray(png::image< png::rgba_pixel_16 > *image)
{
    for(int i = 0; i < n_tile_cols; i++)
    {
        for(int j = 0; j < n_tile_rows; j++)
        {
            tiles[j][i]->savePixelsToPngArray(image);
        }
    }
}

void Surface::clear()
{
    for(int i = 0; i < n_tile_cols; i++)
    {
        for(int j = 0; j < n_tile_rows; j++)
        {
            tiles[j][i]->clear();
        }
    }
}

void Surface::loadImg(QString img_path)
{
    png::image< png::rgba_pixel_16 > image(img_path.toStdString(), png::require_color_space< png::rgba_pixel_16 >());
    for(int i = 0; i < n_tile_cols; i++)
    {
        for(int j = 0; j < n_tile_rows; j++)
        {
            tiles[j][i]->loadImg(&image);
        }
    }
}

void Surface::strokeTo(Brush *brush, qreal x, qreal y, qreal pressure, GLushort dt, bool erase)
{
    qreal dist_between_dabs = brush->getDistBetweenDabsPx();
    qreal stroke_dx = x - last_event_stroke_x;
    qreal stroke_dy = y - last_event_stroke_y;
    last_event_stroke_x = x;
    last_event_stroke_y = y;
    qreal stroke_vx = stroke_dx*50/dt;
    qreal stroke_vy = stroke_dy*50/dt;
    qreal stroke_vel = last_event_stroke_vel*0.8 +
            0.2*sqrt(stroke_vx*stroke_vx + stroke_vy*stroke_vy);
    last_event_stroke_vel = stroke_vel;

    applyXYNoise(brush->getStrokePositionNoisePx(), &previous_stroke_x_noise, &next_stroke_x_noise,
                 &previous_stroke_y_noise, &next_stroke_y_noise, brush->getStrokePositionNoiseFrequency(),
                 &stroke_noise_count, &x, &y);

    qreal dabs_to = countDabsTo(dist_between_dabs, x, y);

    qreal first_dab_dx = x - last_painted_stroke_x;
    qreal first_dab_dy = y - last_painted_stroke_y;;
    qreal dabs_dx = first_dab_dx;
    qreal dabs_dy = first_dab_dy;

    if(isZero(first_dab_dx) && isZero(first_dab_dy))
    {
        first_dab_dx = 0.f;
        first_dab_dy = 0.f;
        dabs_dx = 0.f;
        dabs_dy = 0.f;
    }
    else
    {
        normalize(&first_dab_dx, &first_dab_dy, dist_between_dabs );
        normalize(&dabs_dx, &dabs_dy, dist_between_dabs );
    }
    qreal first_dab_x = last_painted_stroke_x + first_dab_dx;
    qreal first_dab_y = last_painted_stroke_y + first_dab_dy;

    short min_affected_tile_x = 10000;
    short max_affected_tile_x = 0;
    short min_affected_tile_y = 10000;
    short max_affected_tile_y = 0;

    GLshort dabs_to_i = floor(dabs_to);
    if(dabs_to_i > 0)
    {
        qreal rotation_delay = brush->getRotationDelay();
        qreal angle_t = getAngleDeg(x - last_painted_stroke_x, y - last_painted_stroke_y, 0.f, -1.f);
        if(angle_t > 180.f)
        {
            angle_t -= 180.f;
        }
        else if(angle_t < 0.f)
        {
            angle_t += 180.f;
        }
        qreal angle_diff = fabs(angle_t - last_stroke_beta);
        qreal dest_angle;
        if(angle_diff > 120.f)
        {
            dest_angle = angle_t;
        }
        else
        {
            dest_angle = angle_t*(1 - rotation_delay)*brush->getRotationInfluence() +
                                 last_stroke_beta*rotation_delay;
        }

        if(dest_angle > 180.f)
        {
            dest_angle -= 180.f;
        }
        else if(dest_angle < 0.f)
        {
            dest_angle += 180.f;
        }
        last_stroke_beta = dest_angle;
        qreal dab_r = qMin(brush->getRadius()*5,
                             brush->getRadius() + brush->getPressureRadiusGainPx()*pressure +
                             brush->getSpeedRadiusGain()*stroke_vel);
        qreal dab_hardness = brush->getHardness() + brush->getPressureHardnessGain()*pressure +
                                    brush->getSpeedHardnessGain()*stroke_vel;
        qreal dab_opa = brush->getOpacity() + brush->getPressureOpacityGain()*pressure +
                                    brush->getSpeedOpacityGain()*stroke_vel;
        qreal dab_aspect_ratio = brush->getAspectRatio() + brush->getPressureAspectRatioGain()*pressure; // 1 to infinity

        qreal smudge_red;
        qreal smudge_green;
        qreal smudge_blue;
        qreal smudge_alpha;
        getColor(last_painted_stroke_x, last_painted_stroke_y,
                 dab_hardness, dab_opa,
                 dab_aspect_ratio, dab_r, dest_angle,
                 &smudge_red, &smudge_green, &smudge_blue, &smudge_alpha);
        brush->addPickedUpRGBAFromNewStroke(smudge_red, smudge_green, smudge_blue, smudge_alpha);
        brush->getPickedUpRGBA(&smudge_red, &smudge_green, &smudge_blue, &smudge_alpha);


        qreal stroke_red;
        qreal stroke_green;
        qreal stroke_blue;
        qreal dab_alpha;
        bool fixed_color = isZero( brush->getHueNoise() ) && isZero(brush->getSaturationNoise() ) &&
                                isZero( brush->getValueNoise() );

        brush->getRGBA(&stroke_red, &stroke_green, &stroke_blue, &dab_alpha);
        dab_alpha += brush->getPressureAlphaGain()*pressure;

        qreal alpha_sum_t = dab_alpha + smudge_alpha;
        stroke_red = (stroke_red*dab_alpha + smudge_alpha*smudge_red)/alpha_sum_t;
        stroke_green = (stroke_green*dab_alpha + smudge_alpha*smudge_green)/alpha_sum_t;
        stroke_blue = (stroke_blue*dab_alpha + smudge_alpha*smudge_blue)/alpha_sum_t;
        if(alpha_sum_t > 1.f)
        {
            alpha_sum_t = 1.f;
        }

        qreal dab_red = stroke_red;
        qreal dab_green = stroke_green;
        qreal dab_blue = stroke_blue;

        qreal stroke_h;
        qreal stroke_s;
        qreal stroke_v;
        if(!fixed_color)
        {
            stroke_h = stroke_red;
            stroke_s = stroke_green;
            stroke_v = stroke_blue;
            qrgb_to_hsv(&stroke_h, &stroke_s, &stroke_v);
        }

        for(GLshort i = 0; i < dabs_to_i; i++)
        {
            if(!fixed_color)
            {
                dab_red = stroke_h;
                applyNoise(brush->getHueNoise(), &previous_hue_noise, &next_hue_noise,
                           brush->getHueNoiseFrequency(), &hue_noise_count, &dab_red);
                dab_green = stroke_s;
                applyNoise(brush->getSaturationNoise(), &previous_saturation_noise, &next_saturation_noise,
                           brush->getSaturationNoiseFrequency(), &saturation_noise_count, &dab_green);
                dab_blue = stroke_v;
                applyNoise(brush->getValueNoise(), &previous_value_noise, &next_value_noise,
                           brush->getValueNoiseFrequency(), &value_noise_count, &dab_blue);
                qhsv_to_rgb(&dab_red, &dab_green, &dab_blue);
            }
            qreal dab_x = first_dab_x + i*dabs_dx + getNoise(brush->getDabPositionNoisePx() ) ;
            qreal dab_y = first_dab_y + i*dabs_dy + getNoise(brush->getDabPositionNoisePx() ) ;
            last_painted_stroke_x = dab_x;
            last_painted_stroke_y = dab_y;
            last_dab_rotation_inc += brush->getRotationBetweenDabs();
            if(last_dab_rotation_inc > 180.f)
            {
                last_dab_rotation_inc -= 180.f;
            }

            qreal dab_x_min = dab_x - dab_r;
            qreal dab_x_max = dab_x + dab_r;
            qreal dab_y_min = dab_y - dab_r;
            qreal dab_y_max = dab_y + dab_r;

            short dab_min_tile_x;
            short dab_max_tile_x;
            short dab_min_tile_y;
            short dab_max_tile_y;
            getTileIdsOnRect(dab_x_min, dab_x_max, dab_y_min, dab_y_max,
                             &dab_min_tile_x, &dab_max_tile_x, &dab_min_tile_y, &dab_max_tile_y);

            replaceIfSmaller(dab_min_tile_x, &min_affected_tile_x);
            replaceIfSmaller(dab_min_tile_y, &min_affected_tile_y);
            replaceIfBigger(dab_max_tile_x, &max_affected_tile_x);
            replaceIfBigger(dab_max_tile_y, &max_affected_tile_y);
            qreal dab_rot = dest_angle + brush->getInitialRotation() +
                    last_dab_rotation_inc +
                    getNoise(brush->getRotationNoise() );
            #pragma omp parallel for
            for(short tx = dab_min_tile_x; tx <= dab_max_tile_x; tx++)
            {
                for(short ty = dab_min_tile_y; ty <= dab_max_tile_y; ty++)
                {
                    tiles[ty][tx]->addDabToDraw(dab_x, dab_y,
                                                dab_hardness, dab_opa*alpha_sum_t, dab_aspect_ratio,
                                                dab_r, dab_rot,
                                                dab_red, dab_green, dab_blue);
                }
            }
        }

        if(erase)
        {
            for(short tx = min_affected_tile_x; tx <= max_affected_tile_x; tx++)
            {
                for(short ty = min_affected_tile_y; ty <= max_affected_tile_y; ty++)
                {
                    tiles[ty][tx]->processDabErase();
                }
            }
        }
        else
        {
            for(short tx = min_affected_tile_x; tx <= max_affected_tile_x; tx++)
            {
                for(short ty = min_affected_tile_y; ty <= max_affected_tile_y; ty++)
                {
                    tiles[ty][tx]->processDabPainting();
                }
            }
        }

        last_stroke_press = pressure;
    }
}

void Surface::startNewStroke(Brush *brush, qreal x, qreal y, qreal pressure)
{
    stroke_noise_count = UCHAR_MAX;
    previous_stroke_x_noise = 0.f;
    previous_stroke_y_noise = 0.f;
    next_stroke_x_noise = 0.f;
    next_stroke_y_noise = 0.f;

    rotation_noise_count = UCHAR_MAX;
    previous_rotation_noise = 0.f;
    next_rotation_noise = 0.f;

    hue_noise_count = UCHAR_MAX;
    previous_hue_noise = 0.f;
    next_hue_noise = 0.f;

    saturation_noise_count = UCHAR_MAX;
    previous_saturation_noise = 0.f;
    next_saturation_noise = 0.f;

    value_noise_count = UCHAR_MAX;
    previous_value_noise = 0.f;
    next_value_noise = 0.f;

    last_event_stroke_x = x;
    last_event_stroke_y = y;
    last_painted_stroke_x = x;
    last_painted_stroke_y = y;
    last_x_speed_offset = 0.f;
    last_y_speed_offset = 0.f;
    last_stroke_press = pressure;
    brush->resetPickedUpRGBA();

    next_second_color_alpha = 0.f;
    previous_second_color_alpha = 0.f;
    second_color_alpha_count = UCHAR_MAX;
}

Tile *Surface::getTile(ushort tile_col, ushort tile_row)
{
    return tiles[tile_row][tile_col];
}

void Surface::setSize(ushort width_t, ushort height_t)
{
    // initialize tiles
    ushort n_tile_cols_t = ceil(width_t/(double)TILE_DIM);
    ushort n_tile_rows_t = ceil(height_t/(double)TILE_DIM);

    Tile ***tiles_t = new Tile**[n_tile_rows_t];

    for(ushort rw = 0; rw < n_tile_rows_t; rw++)
    {
        tiles_t[rw] = new Tile*[n_tile_cols_t];
        ushort first_new_col_in_row = 0;
        if(rw < n_tile_rows)
        {
            first_new_col_in_row = n_tile_cols;
            for(ushort cl = 0; cl < n_tile_cols; cl++)
            {
                Tile *tile_t = tiles[rw][cl];
                tile_t->resetTileSize();
                tiles_t[rw][cl] = tile_t;
            }
            free(tiles[rw]);
        }

        for(ushort cl = first_new_col_in_row; cl < n_tile_cols_t; cl++)
        { 
            tiles_t[rw][cl] = new Tile(cl*TILE_DIM, rw*TILE_DIM);
        }

    }
    if(tiles != NULL)
    {
        free(tiles);
    }

    tiles = tiles_t;
    width = width_t;
    height = height_t;
    n_tile_rows = n_tile_rows_t;
    n_tile_cols = n_tile_cols_t;


    GLushort last_row_height = height%TILE_DIM;
    if(last_row_height != 0)
    {
        for(int i = 0; i < n_tile_cols; i++)
        {
            tiles[n_tile_rows - 1][i]->setTileHeight(last_row_height);
        }
    }
    GLushort last_column_width = width%TILE_DIM;
    if(last_column_width != 0)
    {
        for(int j = 0; j < n_tile_rows; j++)
        {
            tiles[j][n_tile_cols - 1]->setTileWidth(last_column_width);
        }
    }

}
