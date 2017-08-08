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

class Layer {
public:
    Layer(QString layer_name_t,
          int width_t = 0, int height_t = 0,
          const qreal &scale = 1.,
          const bool &paintInOtherThread = true);

    void clear();

    void tabletEvent(const qreal &x_t,
                     const qreal &y_t,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase,
                     Brush *brush);
    void tabletReleaseEvent();
    void tabletPressEvent(const qreal &x_t,
                          const qreal &y_t,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase, Brush *brush);

    void startStroke(const qreal &x_t,
                     const qreal &y_t,
                     const qreal &pressure,
                     Brush *brush);

    void getTileDrawers(QList<TileSkDrawer*> *tileDrawers) {
        paintlib_surface->getTileDrawers(tileDrawers);
    }
    void setLayerName(const QString &layerName);
    const QString &getLayerName();

    void drawSk(SkCanvas *canvas, SkPaint *paint) {
        paintlib_surface->drawSk(canvas, paint);
    }

    void clearTmp() {
        paintlib_surface->clearTmp();
    }

    void saveToTmp() {
        paintlib_surface->saveToTmp();
    }

    void setSize(const ushort &widthT,
                                const ushort &heightT) {
        paintlib_surface->setSize(widthT, heightT);
    }

private:
    bool mPaintInOtherThread = true;
    QString mLayerName;
    Surface *paintlib_surface = NULL;
    void renderStroke();
};

#endif // LAYER_H
