#include "tile.h"
#include <GLES3/gl3.h>
#include <math.h>
#include <stdio.h>
#include "Colors/helpers.h"

void Tile::savePixelsToPngArray(png::image< png::rgba_pixel_16 > *image)
{
    int id_t = 0;
    png::uint_32 y_min = pos_y;
    png::uint_32 y_max = pos_y + TILE_DIM;
    png::uint_32 x_min = pos_x;
    png::uint_32 x_max = pos_x + TILE_DIM;
    for (png::uint_32 y = y_min; y < y_max; y++)
    {
        for (png::uint_32 x = x_min; x < x_max; x++)
        {
            (*image)[y_min + y_max - y - 1][x] = png::rgba_pixel_16(tex_data[id_t], tex_data[id_t + 1],
                                            tex_data[id_t + 2], tex_data[id_t + 3]);
            id_t += 4;
        }
    }
}

void Tile::clear()
{
    for(int i = 0; i < TILE_DIM*TILE_DIM*4; i++)
    {
        tex_data[i] = 0;
    }
    updateTexFromDataArray();
}

void Tile::loadImg(png::image<png::rgba_pixel_16> *image)
{
    int id_t = 0;
    png::uint_32 y_min = pos_y;
    png::uint_32 y_max = pos_y + TILE_DIM;
    png::uint_32 x_min = pos_x;
    png::uint_32 x_max = pos_x + TILE_DIM;
    for (png::uint_32 y = y_min; y < y_max; y++)
    {
        for (png::uint_32 x = x_min; x < x_max; x++)
        {
            png::rgba_pixel_16 pixel_t = image->get_pixel(x, y_min + y_max - y - 1);
            tex_data[id_t] = pixel_t.red;
            tex_data[id_t + 1] = pixel_t.green;
            tex_data[id_t + 2] = pixel_t.blue;
            tex_data[id_t + 3] = pixel_t.alpha;
            id_t += 4;
        }
    }
    updateTexFromDataArray();
}

void Tile::setTileWidth(GLushort width_t)
{
    max_paint_x = width_t;
}

void Tile::setTileHeight(GLushort height_t)
{
    min_paint_y = TILE_DIM - height_t;
}

void Tile::resetTileSize()
{
    max_paint_x = TILE_DIM;
    min_paint_y = 0;
}

Tile::Tile(ushort x_t, ushort y_t)
{
    setPosInSurface(x_t, y_t);
    genFrameBuffersAndTextures();
    clear();
}

Tile::~Tile()
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glDeleteFramebuffers( 1, &disp_buffer_id );
}

void CHECK_FRAMEBUFFER_STATUS()
{
    GLenum status;
    status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE:
        break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
    /* choose different formats */
        break;

    default:
        /* programming error; will fail on all hardware */
        fputs("Framebuffer Error\n", stderr);
        //exit(-1);
    }
}
#include <QDebug>
void Tile::genFramebufferAndTexture(GLuint *buffer_t, GLuint *render_buffer_t,
                                    GLuint *texture_t)
{
    glGenFramebuffers(1, buffer_t);
    glBindFramebuffer(GL_FRAMEBUFFER, *buffer_t);
    glGenRenderbuffers(1, render_buffer_t);

    glGenTextures(1, texture_t);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, *texture_t);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TILE_DIM, TILE_DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture_t, 0 );

    glBindRenderbuffer(GL_RENDERBUFFER, *render_buffer_t);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, TILE_DIM, TILE_DIM);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *render_buffer_t);

    CHECK_FRAMEBUFFER_STATUS();

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Tile::setPosInSurface(ushort x_t, ushort y_t)
{
    pos_x = x_t;
    pos_y = y_t;
}

void Tile::paintGL()
{
    glPushMatrix();
    glTranslatef(pos_x, -pos_y, 0.f);

    glBindTexture( GL_TEXTURE_2D, disp_tex_id );
    glBegin( GL_QUADS );
        glTexCoord2f( 0.f, 1.f ); glVertex2f( 0.f, 0.f );
        glTexCoord2f( 1.f, 1.f ); glVertex2f( TILE_DIM, 0.f );
        glTexCoord2f( 1.f, 0.f ); glVertex2f( TILE_DIM, -TILE_DIM );
        glTexCoord2f( 0.f, 0.f ); glVertex2f( 0.f, -TILE_DIM);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

void Tile::genFrameBuffersAndTextures()
{
    genFramebufferAndTexture(&disp_buffer_id, &disp_render_buffer_id, &disp_tex_id);
    tex_data = new GLushort[TILE_DIM*TILE_DIM*4];
}

void Tile::partOfSurfRectInTile(short surf_x, short surf_y, short width, short height,
                                short *rect_x, short *rect_y, short *width_t, short *height_t)
{
    if(surf_x > pos_x)
    {
        *rect_x = surf_x - pos_x;
        *width_t = width;
        if(*rect_x + width > TILE_DIM)
        {
            *width_t = TILE_DIM - *rect_x;
        }
        else
        {
            *width_t = width;
        }
    }
    else
    {
        *rect_x = 0;
        *width_t = width - (pos_x - surf_x);
        if(*width_t > TILE_DIM)
        {
            *width_t = TILE_DIM;
        }
    }
    if(surf_y > pos_y)
    {
        *rect_y = surf_y - pos_y;
        *height_t = height;
        if(*rect_y + height > TILE_DIM)
        {
            *height_t = TILE_DIM - *rect_y;
        }
        else
        {
            *width_t = width;
        }
    }
    else
    {
        *rect_y = 0;
        *height_t = height - (pos_y - surf_y);
        if(*height_t > TILE_DIM)
        {
            *height_t = TILE_DIM;
        }
    }
}

bool Tile::surfRectInTile(int surf_x, int surf_y, int width, int height)
{
    if(surf_x > pos_x + TILE_DIM)
    {
        return false;
    }
    if(surf_y > pos_y + TILE_DIM)
    {
        return false;
    }
    if(surf_x + width < pos_x)
    {
        return false;
    }
    if(surf_y + height < pos_y)
    {
        return false;
    }
    return true;
}

void Tile::addDabToDraw(qreal cx, qreal cy,
                       qreal hardness, qreal opa,
                       qreal aspect_ratio, qreal r, qreal beta_deg,
                       qreal red, qreal green, qreal blue)
{
    cx -= pos_x;
    cy -= pos_y;
    cy = TILE_DIM - cy;

    dabs_to_paint.append(Dab(cx, cy, hardness, opa, aspect_ratio, r, beta_deg, red, green, blue) );
}

void Tile::getColor(qreal cx, qreal cy,
                    qreal r, qreal aspect_ratio, qreal cs, qreal sn,
                    qreal hardness, qreal opacity,
                    qreal *red_sum, qreal *green_sum, qreal *blue_sum, qreal *alpha_sum,
                    qreal *weight_sum)
{
    cx -= pos_x;
    cy -= pos_y;

    int x_min = floor(cx - r);
    if(x_min > max_paint_x)
    {
        return;
    }
    else if(x_min < 0)
    {
        x_min = 0;
    }
    int x_max = ceil(cx + r);
    if(x_max > max_paint_x)
    {
        x_max = max_paint_x;
    }
    else if(x_max < 0)
    {
        x_max = 0;
    }
    int y_min = floor(cy - r);
    if(y_min > TILE_DIM)
    {
        return;
    }
    else if(y_min < min_paint_y)
    {
        y_min = min_paint_y;
    }
    int y_max = ceil(cy + r);
    if(y_max > TILE_DIM)
    {
        y_max = TILE_DIM;
    }
    else if(y_max < min_paint_y)
    {
        y_max = min_paint_y;
    }

    qreal red_sum_t = 0.f;
    qreal green_sum_t = 0.f;
    qreal blue_sum_t = 0.f;
    qreal alpha_sum_t = 0.f;
    qreal weight_sum_t = 0.f;
    #pragma omp parallel for reduction(+: red_sum_t, green_sum_t, blue_sum_t, alpha_sum_t, weight_sum_t)
    for(short i = x_min; i < x_max; i++)
    {
        for(short j = y_min; j < y_max; j++)
        {
            GLuint col_val_id = ( (TILE_DIM - j - 1 + min_paint_y)*TILE_DIM + i)*4;
            qreal dx = i - cx;
            qreal dy = j - cy;
            qreal dyr = (dy*cs - dx*sn)*aspect_ratio;
            qreal dxr = (dy*sn + dx*cs);
            qreal curr_r_frac = (dyr*dyr + dxr*dxr) / (r*r);
            qreal h_opa;
            if(curr_r_frac > 1)
            {
                h_opa = 0;
            }
            else if(curr_r_frac < hardness)
            {
                h_opa = curr_r_frac + 1 - curr_r_frac/hardness;
            }
            else
            {
                h_opa = hardness/(1 - hardness)*(1 - curr_r_frac);
            }
            qreal alpha_t = tex_data[col_val_id + 3]/(float)USHRT_MAX;
            qreal weight_t = opacity * h_opa;
            weight_sum_t += weight_t;
            qreal alpha_sum_inc_t = weight_t*alpha_t;
            red_sum_t += tex_data[col_val_id]/(float)USHRT_MAX*alpha_sum_inc_t;
            green_sum_t += tex_data[col_val_id + 1]/(float)USHRT_MAX*alpha_sum_inc_t;
            blue_sum_t += tex_data[col_val_id + 2]/(float)USHRT_MAX*alpha_sum_inc_t;
            alpha_sum_t += alpha_sum_inc_t;
        }
    }

    *red_sum += red_sum_t;
    *green_sum += green_sum_t;
    *blue_sum += blue_sum_t;
    *alpha_sum += alpha_sum_t;
    *weight_sum += weight_sum_t;
}

void Tile::processDabPainting()
{
    foreach(Dab dab_t, dabs_to_paint)
    {
        qreal cs = cos(dab_t.beta_deg*2*PI/360);
        qreal sn = sin(dab_t.beta_deg*2*PI/360);


        int x_min = floor(dab_t.cx - dab_t.r);
        if(x_min > max_paint_x)
        {
            continue;
        }
        else if(x_min < 0)
        {
            x_min = 0;
        }
        int x_max = ceil(dab_t.cx + dab_t.r);
        if(x_max > max_paint_x)
        {
            x_max = max_paint_x;
        }
        else if(x_max < 0)
        {
            x_max = 0;
        }
        int y_min = floor(dab_t.cy - dab_t.r);
        if(y_min > TILE_DIM)
        {
            continue;
        }
        else if(y_min < min_paint_y)
        {
            y_min = min_paint_y;
        }
        int y_max = ceil(dab_t.cy + dab_t.r);
        if(y_max > TILE_DIM)
        {
            y_max = TILE_DIM;
        }
        else if(y_max < min_paint_y)
        {
            y_max = min_paint_y;
        }

        #pragma omp parallel for
        for(int i = x_min; i < x_max; i++)
        {
            for(int j = y_min; j < y_max; j++)
            {
                GLuint id_t = ( j*TILE_DIM + i)*4;
                qreal dx = i - dab_t.cx;
                qreal dy = j - dab_t.cy;
                qreal dyr = (dy*cs - dx*sn)*dab_t.aspect_ratio;
                qreal dxr = (dy*sn + dx*cs);
                qreal curr_r_frac = (dyr*dyr + dxr*dxr) / (dab_t.r*dab_t.r);
                qreal h_opa;
                if(curr_r_frac > 1)
                {
                    h_opa = 0;
                }
                else if(curr_r_frac < dab_t.hardness)
                {
                    h_opa = curr_r_frac + 1 - curr_r_frac/dab_t.hardness;
                }
                else
                {
                    h_opa = dab_t.hardness/(1 - dab_t.hardness)*(1 - curr_r_frac);
                }
                qreal curr_alpha = tex_data[id_t + 3];
                qreal paint_alpha = dab_t.opacity * h_opa * USHRT_MAX;
                qreal alpha_sum = curr_alpha + paint_alpha;
                // red
                tex_data[id_t] = (dab_t.red*USHRT_MAX*paint_alpha + tex_data[id_t]*curr_alpha)/alpha_sum;
                // green
                tex_data[id_t + 1] = (dab_t.green*USHRT_MAX*paint_alpha + tex_data[id_t + 1]*curr_alpha)/alpha_sum;
                // blue
                tex_data[id_t + 2] = (dab_t.blue*USHRT_MAX*paint_alpha + tex_data[id_t + 2]*curr_alpha)/alpha_sum;
                // alpha
                if(alpha_sum > USHRT_MAX)
                {
                    tex_data[id_t + 3] = USHRT_MAX;
                }
                else
                {
                    tex_data[id_t + 3] = alpha_sum;
                }
            }
        }
    }

    updateTexFromDataArray();

    dabs_to_paint.clear();
}

void Tile::processDabErase()
{
    foreach(Dab dab_t, dabs_to_paint)
    {
        qreal cs = cos(dab_t.beta_deg*2*PI/360);
        qreal sn = sin(dab_t.beta_deg*2*PI/360);


        int x_min = floor(dab_t.cx - dab_t.r);
        if(x_min > max_paint_x)
        {
            continue;
        }
        else if(x_min < 0)
        {
            x_min = 0;
        }
        int x_max = ceil(dab_t.cx + dab_t.r);
        if(x_max > max_paint_x)
        {
            x_max = max_paint_x;
        }
        else if(x_max < 0)
        {
            x_max = 0;
        }
        int y_min = floor(dab_t.cy - dab_t.r);
        if(y_min > TILE_DIM)
        {
            continue;
        }
        else if(y_min < min_paint_y)
        {
            y_min = min_paint_y;
        }
        int y_max = ceil(dab_t.cy + dab_t.r);
        if(y_max > TILE_DIM)
        {
            y_max = TILE_DIM;
        }
        else if(y_max < min_paint_y)
        {
            y_max = min_paint_y;
        }

        #pragma omp parallel for
        for(int i = x_min; i < x_max; i++)
        {
            for(int j = y_min; j < y_max; j++)
            {
                GLuint id_t = ( j*TILE_DIM + i)*4;
                qreal dx = i - dab_t.cx;
                qreal dy = j - dab_t.cy;
                qreal dyr = (dy*cs - dx*sn)*dab_t.aspect_ratio;
                qreal dxr = (dy*sn + dx*cs);
                qreal curr_r_frac = (dyr*dyr + dxr*dxr) / (dab_t.r*dab_t.r);
                qreal h_opa;
                if(curr_r_frac > 1)
                {
                    h_opa = 0;
                }
                else if(curr_r_frac < dab_t.hardness)
                {
                    h_opa = curr_r_frac + 1 - curr_r_frac/dab_t.hardness;
                }
                else
                {
                    h_opa = dab_t.hardness/(1 - dab_t.hardness)*(1 - curr_r_frac);
                }
                qreal curr_alpha = tex_data[id_t + 3];
                qreal paint_alpha = dab_t.opacity * h_opa * USHRT_MAX;
                qreal alpha_sum = curr_alpha - paint_alpha;

                if(alpha_sum < 0)
                {
                    tex_data[id_t + 3] = 0;
                }
                else
                {
                    tex_data[id_t + 3] = alpha_sum;
                }
            }
        }
    }

    updateTexFromDataArray();

    dabs_to_paint.clear();
}

void Tile::updateDataArrayFromTex()
{
    glBindTexture( GL_TEXTURE_2D, disp_tex_id);
    glGetTexImage(GL_TEXTURE_2D, 0,GL_RGBA, GL_UNSIGNED_SHORT, tex_data);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Tile::updateTexFromDataArray()
{
    glBindTexture( GL_TEXTURE_2D, disp_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TILE_DIM, TILE_DIM,
                 0, GL_RGBA, GL_UNSIGNED_SHORT, tex_data);
    glBindTexture(GL_TEXTURE_2D, 0);
}
