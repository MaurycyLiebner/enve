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

struct WindowVariables;

class Canvas;

class CanvasHandler : public QObject
{
    Q_OBJECT
public:
    CanvasHandler(WindowVariables *window_vars_t, int width_t, int height_t,
                  bool main_canvas_t);
    void newLayer(QString layer_name_t);
    void removeLayer(Layer *layer_t);
    void paintGL();

    void incNumberItems();
    void decNumberItems();

    void clear();

    void tabletEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, float pressure, bool erase);
    void tabletReleaseEvent();
    void tabletPressEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, float pressure);

    void setCurrentLayer(int layer_id);
    void setCurrentLayer(Layer *layer_t);
    void addLayer(Layer *layer_t);
    void changeLayerZFromTo(int z_val_t, int z_new_val_t);
    void mouseReleaseEvent();
    void mousePressEvent(GLfloat x_t, GLfloat y_t, ulong timestamp, float pressure);
    void mousePaintEvent(GLfloat x_t, GLfloat y_t, ulong time_stamp, bool erase);
    void saveAsPng(QString file_name);
    void drawGridBg();
    void drawColorBg();
    void drawStretchedImgBg();
    void drawRepeatedImgBg();
    void setBackgroundMode(CanvasBackgroundMode bg_mode_t);
    void backgroundImgFromFile(QString file_name);
    void setBackgroundColorRGB(GLfloat r_t, GLfloat g_t, GLfloat b_t);
    void setBackgroundColorHSV(GLfloat h_t, GLfloat s_t, GLfloat v_t);

    bool connectedToTreeWidget();

    void repaint();

    void loadImage(int layer_id_t, QString img_path_t);

    int getLayersCount();
    void replaceLayerImage(int layer_id_t, QString img_path_t);
private:

    bool main_canvas = true;

    CanvasBackgroundMode backgroud_mode = CANVAS_BACKGROUND_COLOR;
    Color background_color = Color(1.f, 1.f, 1.f);
    GLuint background_img = 0;

    WindowVariables *window_vars = NULL;

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
