#ifndef CANVASHANDLER_H
#define CANVASHANDLER_H
#include <QList>
#include "layer.h"
#include "Colors/color.h"

enum CanvasBackgroundMode
{
    CANVAS_BACKGROUND_COLOR,
    CANVAS_BACKGROUND_STRETCHED_IMAGE,
    CANVAS_BACKGROND_REPEATED_IMAGE,
    CANVAS_BACKGROUND_GRID
};

class Canvas;

class CanvasHandler {
public:
    CanvasHandler(int width_t, int height_t);
    void newLayer(QString layer_name_t);
    void removeLayer(Layer *layer_t);

    void incNumberItems();
    void decNumberItems();

    void clear();

    void tabletEvent(qreal xT,
                     qreal yT,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase);
    void tabletReleaseEvent();
    void tabletPressEvent(qreal xT,
                          qreal yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          Brush *brush);

    void setCurrentLayer(int layer_id);
    void setCurrentLayer(Layer *layer_t);
    void addLayer(Layer *layer_t);
    void changeLayerZFromTo(int z_val_t, int z_new_val_t);
    void mouseReleaseEvent();
    void mousePressEvent(qreal xT,
                         qreal yT,
                         const ulong &timestamp,
                         const qreal &pressure, Brush *brush);
    void mouseMoveEvent(qreal xT,
                        qreal yT,
                        const ulong &time_stamp,
                        const bool &erase);
    void drawGridBg();
    void drawColorBg();
    void drawStretchedImgBg();
    void drawRepeatedImgBg();
    void setBackgroundMode(CanvasBackgroundMode bg_mode_t);
    void backgroundImgFromFile(QString file_name);
    void setBackgroundColorRGB(const qreal &r_t,
                               const qreal &g_t,
                               const qreal &b_t);
    void setBackgroundColorHSV(const qreal &h_t,
                               const qreal &s_t,
                               const qreal &v_t);

    bool connectedToTreeWidget();

    int getLayersCount();

    void getTileDrawers(QList<TileSkDrawer> *tileDrawers);
    void paintPress(qreal xT,
                    qreal yT,
                    const ulong &timestamp,
                    const qreal &pressure,
                    Brush *brush);
private:
    virtual void mapToPaintCanvasHandler(qreal *x_t, qreal *y_t) = 0;

    CanvasBackgroundMode backgroud_mode = CANVAS_BACKGROUND_COLOR;
    Color background_color = Color(1.f, 1.f, 1.f);
    GLuint background_img = 0;

    int width = 0;
    int height = 0;
    Layer *current_layer = NULL;
    uint16_t *img = NULL;
    bool load_img = false;
    int16_t n_items = 0;
    int8_t n_layers = 0;
    QList<Layer*> layers;
};

#endif // CANVASHANDLER_H
