#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "canvas.h"
#include "undoredo.h"
#include "Colors/color.h"
#include <QDockWidget>

class UpdateScheduler;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QDockWidget *mRightDock;

    QToolBar *mToolBar;
    QAction *mActionConnectPoints;
    QAction *mActionDisconnectPoints;
    QAction *mActionMergePoints;
//
    QAction *mActionSymmetricPointCtrls;
    QAction *mActionSmoothPointCtrls;
    QAction *mActionCornerPointCtrls;
//

    Canvas *mCanvas;
    UndoRedoStack mUndoRedoStack;
    QList<UpdateScheduler*> mUpdateSchedulers;
protected:
    void keyPressEvent(QKeyEvent *event);
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(UpdateScheduler *scheduler);

    void callUpdateSchedulers();
    void scheduleRepaint();

    static Color getColor(bool darker, bool selected, bool hover);
// colors
    static Color color_hover;
    static Color color_hover_darker;
    static Color color_selected;
    static Color color_selected_darker;
    static Color color_selected_hover;
    static Color color_selected_hover_darker;
    static Color color_current;
    static Color color_current_darker;
    static Color color_current_hover;
    static Color color_current_hover_darker;
    static Color color_light;
    static Color color_darker;
    static Color color_dark;
    static Color color_selected_active;
};

#endif // MAINWINDOW_H
