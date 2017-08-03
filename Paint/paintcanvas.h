#ifndef CANVAS_H
#define CANVAS_H
#include <QOpenGLWidget>
#include <GL/glu.h>
#include <GL/gl.h>

class CanvasHandler;

struct PaintCanvasRect
{
    GLfloat x = 0.0f;
    GLfloat y = 0.0f;
    GLfloat width = 0.0f;
    GLfloat height = 0.0f;
};

struct WindowVariables;

class PaintCanvas : public QOpenGLWidget
{
public:
    PaintCanvas(ushort canvas_width_t, ushort canvas_height_t, bool main_canvas_t,
           WindowVariables *window_vars_t, QWidget *parent = 0);
    ~PaintCanvas();

    void setTileSize(quint16 width_t, quint16 height_t);
    PaintCanvasRect getVisibleRect();

    GLfloat getVisibleX();
    GLfloat getVisibleY();
    GLfloat getVisibleWidth();
    GLfloat getVisibleHeight();

    uint16_t getTileWidth();
    uint16_t getTileHeight();
    void saveMoveEventVars(int x_t, int y_t);
    void paintPress(GLfloat x_t, GLfloat y_t, ulong timestamp, float pressure);

    void updateCursor(int radius);

    bool isZoomingEnabled();
    bool isMovingEnabled();
    void setZoomingEnabled(bool enabled_t);
    void setMovingEnabled(bool enabled_t);
    void saveAsPng(QString file_name);

    void clear();

    void loadImage(int id_t, QString img_path);


    void resetZoom();
    void zoomCenter(GLfloat ds);
    void moveVisiblePos(GLfloat dx, GLfloat dy);
    void zoom(GLfloat ds, GLfloat center_x, GLfloat center_y);
    void replaceLayerImage(int layer_id_t, QString img_path_t);

    int getLayersCount();
private:
    bool img_load_queued = false;
    int queued_load_layer_id;
    QString queued_load_img;

    bool initialized = false;

    bool main_canvas;

    ushort canvas_width = 0;
    ushort canvas_height = 0;

    bool zooming_enabled = true;
    bool moving_enabled = true;

    WindowVariables *window_vars;
    bool middle_button_pressed = false;
    bool stylus_drawing = false;
    // tiles variables

    PaintCanvasRect visible_rect;

    GLfloat gl_trans_x = 0.0f;
    GLfloat gl_trans_y = 0.0f;
    GLfloat scale = 1.0f;

    GLfloat press_v_r_x = 0.0f;
    GLfloat press_v_r_y = 0.0f;

    GLfloat mouse_x = 0.0f;
    GLfloat mouse_y = 0.0f;

    int m_track_x = 0;
    int m_track_y = 0;

    void moveVisibleRect(GLfloat dx, GLfloat dy);
    void centerVisibleRectOn(GLfloat x_c_t, GLfloat y_c_t);
    void setVisbilePos(GLfloat x_t, GLfloat y_t);
    void updateVisibleRectSize();

    void mapFromWidgetToPaintCanvasHandler(GLfloat *x_t, GLfloat *y_t);
    void mapFromWidgetToPaintCanvas(GLfloat *x_t, GLfloat *y_t);
    void updateTileLinesWidth();
    void drawGridBg();
    void drawColorBg();
    void drawStretchedImgBg();
    void drawRepeatedImgBg();

protected:
    GLfloat absHeightToGLHeight(GLfloat height_t);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *e);
    void tabletEvent(QTabletEvent *e);
    CanvasHandler *canvas_handler = NULL;
};

#endif // CANVAS_H
