#ifndef LAYER_H
#define LAYER_H
#include "GL/gl.h"
#include <QString>
#include "PaintLib/surface.h"
#include <QList>
#include <QTimer>
#include <QObject>
#include <QColor>
#include <png++/png.hpp>

class TreeWidgetItem;

class CanvasHandler;

struct WindowVariables;

struct LayerRenderData
{
    int n_tex = 0;
    GLuint *tile_render_data = NULL;
    int n_tile_rows = 0;
    int n_tile_cols = 0;
    int tile_size = 0;
};

class Layer : public QObject
{
    Q_OBJECT
public:
    Layer(QString layer_name_t,
          CanvasHandler *canvas_handler_t,
          WindowVariables *window_vars_t,
          int width_t = 0, int height_t = 0);

    void paint();

    void clear();
    void setX(int16_t x_t);
    void setY(int16_t y_t);
    void moveBy(int16_t dx, int16_t dy);
    void setPos(int16_t x_t, int16_t y_t);

    void setWidth(uint16_t width_t);
    void setHeight(uint16_t height_t);
    void setSize(uint16_t width_t, uint16_t height_t);

    void setLayerName(QString layer_name_t);
    QString getLayerName();

    void tabletEvent(GLfloat x_t, GLfloat y_t,
                     ulong time_stamp,
                     float pressure, bool erase);
    void tabletReleaseEvent();
    void tabletPressEvent(GLfloat x_t, GLfloat y_t,
                          ulong time_stamp, float pressure);

    void startStroke(GLfloat x_t, GLfloat y_t, GLfloat pressure);

    void setNewPaintLibSurface(int width_t, int height_t);

    void repaint();

    void setLayerFilePath(QString path_t);

    void getTileDrawers(QList<TileSkDrawer> *tileDrawers) {
        paintlib_surface->getTileDrawers(tileDrawers);
    }

private:
    QString layer_file_path;
    WindowVariables *window_vars = NULL;
    CanvasHandler *canvas_handler = NULL;
    ushort time_repaint = 10;
    bool painting = false;
    Surface *paintlib_surface = NULL;
    //GLushort *data = NULL;
    uint16_t *data = NULL;
    QString layer_name = "";
    uint16_t layer_width = 0;
    uint16_t layer_height = 0;
public slots:
    void renderStroke();
};

#endif // LAYER_H
