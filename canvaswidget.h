#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
class Canvas;
class FillStrokeSettingsWidget;
enum CanvasMode : short;

class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = 0);

    Canvas *getCurrentCanvas();

    void setCurrentCanvas(Canvas *canvas);
    void addCanvasToList(Canvas *canvas);
    void removeCanvas(const int &id);
    void addCanvasToListAndSetAsCurrent(Canvas *canvas);
    void renameCanvas(Canvas *canvas, const QString &newName);
    void renameCanvas(const int &id, const QString &newName);
    bool hasNoCanvas();
    void setCanvasMode(const CanvasMode &mode);

    void callUpdateSchedulers();
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);
protected:
    Canvas *mCurrentCanvas = NULL;
    QList<Canvas*> mCanvasList;

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
signals:

public slots:
    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();

    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int &id);
};

#endif // CANVASWIDGET_H
