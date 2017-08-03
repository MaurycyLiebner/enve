#ifndef TILE_H
#define TILE_H

#define TILE_DIM 64

#include <QList>
#include <stdlib.h>
#include <GL/gl.h>
#include "Colors/helpers.h"
#include <png++/png.hpp>

struct Dab
{
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
    Dab(qreal cx_t, qreal cy_t, qreal hardness_t, qreal opacity_t,
        qreal aspect_ratio_t, qreal r_t, qreal beta_t, qreal red_t, qreal green_t, qreal blue_t)
    {
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
    }
};

class Tile
{
public:
    Tile(ushort x_t, ushort y_t);
    ~Tile();

    void paintGL();
    void genFrameBuffersAndTextures();
    void partOfSurfRectInTile(short surf_x, short surf_y, short width, short height,
                              short *rect_x, short *rect_y, short *width_t, short *height_t);
    bool surfRectInTile(int surf_x, int surf_y, int width, int height);


    void processDabPainting();
    void addDabToDraw(qreal cx, qreal cy,
                 qreal hardness, qreal opa,
                 qreal aspect_ratio, qreal r, qreal beta,
                 qreal red, qreal green, qreal blue);
    void updateDataArrayFromTex();
    void updateTexFromDataArray();
    void getColor(qreal cx, qreal cy, qreal r,
                  qreal aspect_ratio, qreal cs, qreal sn, qreal hardness, qreal opacity,
                  qreal *red_sum, qreal *green_sum, qreal *blue_sum, qreal *alpha_sum,
                  qreal *weight_sum);
    void savePixelsToPngArray(png::image<png::rgba_pixel_16> *image);

    void clear();

    void loadImg(png::image< png::rgba_pixel_16 > *image);


    void setTileWidth(GLushort width_t);
    void setTileHeight(GLushort height_t);
    void resetTileSize();
    void processDabErase();
private:
    QList<Dab> dabs_to_paint;

    void mergePaintTexWithDispTex();
    void drawDabSegTris(qreal x1, qreal y1, qreal x2, qreal y2, qreal cx, qreal cy,
                        qreal r12, qreal g12, qreal b12, qreal a12,
                        qreal rc, qreal gc, qreal bc, qreal ac);
    void drawDabSegQuad(qreal x11, qreal y11, qreal x12, qreal y12,
                        qreal x21, qreal y21, qreal x22, qreal y22,
                        qreal r1, qreal g1, qreal b1, qreal a1,
                        qreal r2, qreal g2, qreal b2, qreal a2);
    GLushort *getSubTexData(GLuint buffer_id, GLuint x, GLuint y, GLuint width, GLuint height);

    void setPosInSurface(ushort x_t, ushort y_t);
    void genFramebufferAndTexture(GLuint *buffer_t, GLuint *render_buffer_t, GLuint *texture_t);

    ushort min_paint_y = 0;

    // position in surface
    ushort pos_x = 0;
    ushort pos_y = 0;

// display texture
    GLuint disp_tex_id = 0;
    GLuint disp_buffer_id = 0;
    GLuint disp_render_buffer_id = 0;
    GLushort *tex_data = NULL;

    GLushort max_paint_x = TILE_DIM;
};

#endif // TILE_H
