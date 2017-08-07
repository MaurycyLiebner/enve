#ifndef SURFACE_H
#define SURFACE_H
#include <stdlib.h>
#include "tile.h"
#include "brush.h"
#include <png++/png.hpp>

struct Stroke {
    Brush *brush = NULL;
    qreal x;
    qreal y;
    qreal pressure;
    ushort dt;
    Stroke(Brush *brush_t,
           qreal x_t, qreal y_t,
           qreal pressure_t,
           ushort dt_t) {
        brush = brush_t;
        x = x_t;
        y = y_t;
        pressure = pressure_t;
        dt = dt_t;
    }
};

class Surface {
public:
    Surface(const ushort &width_t, const ushort &height_t,
            const bool &paintOnOtherThread = true);
    void strokeTo(Brush *brush,
                  qreal x, qreal y,
                  qreal pressure, GLushort dt,
                  bool erase);
    void startNewStroke(Brush *brush,
                        qreal x, qreal y,
                        qreal pressure);
    void getColor(qreal cx, qreal cy,
                  qreal hardness, qreal opa,
                  qreal aspect_ratio, qreal r, qreal beta_deg,
                  qreal *red, qreal *green, qreal *blue, qreal *alpha);

    void clear();

    void getTileDrawers(QList<TileSkDrawer *> *tileDrawers);

    void drawSk(SkCanvas *canvas, SkPaint *paint) {
        for(int i = 0; i < n_tile_cols; i++) {
            for(int j = 0; j < n_tile_rows; j++) {
                tiles[j][i]->drawSk(canvas, paint);
            }
        }
    }

private:
    bool mPaintInOtherThread = true;
    qreal countDabsTo(qreal dist_between_dabs, qreal x, qreal y);
    Tile *getTile(ushort tile_col, ushort tile_row);
    void setSize(ushort width_t, ushort height_t);

    qreal previous_second_color_alpha = 0.f;
    qreal next_second_color_alpha = 0.f;
    uchar second_color_alpha_count = UCHAR_MAX;

    qreal last_dab_rotation_inc = 0.f;
    qreal last_stroke_beta = 0.f;

    uchar stroke_noise_count = UCHAR_MAX;
    qreal previous_stroke_x_noise = 0.f;
    qreal previous_stroke_y_noise = 0.f;
    qreal next_stroke_x_noise = 0.f;
    qreal next_stroke_y_noise = 0.f;

    uchar rotation_noise_count = UCHAR_MAX;
    qreal previous_rotation_noise = 0.f;
    qreal next_rotation_noise = 0.f;

    uchar hue_noise_count = UCHAR_MAX;
    qreal previous_hue_noise = 0.f;
    qreal next_hue_noise = 0.f;

    uchar saturation_noise_count = UCHAR_MAX;
    qreal previous_saturation_noise = 0.f;
    qreal next_saturation_noise = 0.f;

    uchar value_noise_count = UCHAR_MAX;
    qreal previous_value_noise = 0.f;
    qreal next_value_noise = 0.f;

    qreal last_x_speed_offset = 0.f;
    qreal last_y_speed_offset = 0.f;

    qreal last_event_stroke_x = 0.f;
    qreal last_event_stroke_y = 0.f;
    qreal last_event_stroke_vel = 0.f;

    qreal last_painted_stroke_x = 0.f;
    qreal last_painted_stroke_y = 0.f;
    qreal last_stroke_press = 0.f;

    ushort width = 0;
    ushort height = 0;
    ushort n_tile_cols = 0;
    ushort n_tile_rows = 0;
    Tile ***tiles = NULL;
    void getTileIdsOnRect(qreal x_min, qreal x_max, qreal y_min, qreal y_max,
                          short *tile_x_min, short *tile_x_max, short *tile_y_min, short *tile_y_max);
};

#endif // SURFACE_H
