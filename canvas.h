#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "undoredo.h"
#include "vectorpath.h"

class MainWindow;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT
};

class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas(MainWindow *parent = 0);
    void callKeyPress(QKeyEvent *event);
    void addPathToSelection(VectorPath *path);
    void clearPathsSelection();
    void selectOnlyLastPressedPath();
    void removePointFromSelection(PathPoint *point);
    void removePathFromSelection(VectorPath *path);
    void selectOnlyLastPressedPoint();
    bool isShiftPressed();
    void connectPointsFromDifferentPaths(PathPoint *point1, PathPoint *pointDest);

    void addPath(VectorPath *path);
    void removePath(VectorPath *path);
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void clearPointsSelection();
    void addPointToSelection(PathPoint *point);

    void setCurrentPoint(PathPoint *point);

    PathPoint *getCurrentPoint();

    void addUndoRedo(UndoRedo undoRedo);
signals:

public slots:

private:
    MainWindow *mMainWindow;
    bool mMouseClick = false;
    bool mSelecting = false;
//    bool mMoving = false;
    QPoint mPressPos;
    QRectF mSelectionRect;
    CanvasMode mCurrentMode = ADD_POINT;
    QList<VectorPath*> mPaths;
    PathPoint *mLastPressedPoint = NULL;
    PathPoint *mCurrentPoint = NULL;
    QList<PathPoint*> mSelectedPoints;
    VectorPath *mLastPressedPath = NULL;
    QList<VectorPath*> mSelectedPaths;
};

#endif // CANVAS_H
