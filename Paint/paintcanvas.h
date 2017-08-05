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

class PaintCanvas {
public:
    PaintCanvas(const ushort &canvas_width_t,
                const ushort &canvas_height_t);

    void clear();

    int getLayersCount();
private:
    WindowVariables *window_vars;
    bool mStylusDrawing = false;
    // tiles variables

    virtual void mapToPaintCanvasHandler(qreal *x_t, qreal *y_t) = 0;
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
    void tabletEvent(QTabletEvent *e);
    void paintPress(const qreal &x_t,
                    const qreal &y_t,
                    const ulong &timestamp,
                    const qreal &pressure);

    CanvasHandler *canvas_handler = NULL;
};

#endif // CANVAS_H
