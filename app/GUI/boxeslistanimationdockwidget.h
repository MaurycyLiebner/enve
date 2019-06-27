#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <qscrollarea.h>
#include <QScrollArea>
#include <QApplication>
#include <QScrollBar>
#include <QComboBox>
#include <QMenuBar>
#include <QLineEdit>
#include <QWidgetAction>
#include <QToolBar>
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"
#include "timelinelayout.h"
class FrameScrollBar;
class TimelineWidget;
class VWidgetStack;
class RenderWidget;
class MainWindow;
class AnimationDockWidget;
class RenderWidget;
class ActionButton;
class Canvas;
class Document;
class LayoutHandler;

class BoxesListAnimationDockWidget : public QWidget {
public:
    explicit BoxesListAnimationDockWidget(Document &document,
                                          LayoutHandler* const layoutH,
                                          MainWindow *parent);
    bool processKeyEvent(QKeyEvent *event);
    void previewFinished();
    void previewBeingPlayed();
    void previewBeingRendered();
    void previewPaused();

    void updateSettingsForCurrentCanvas(Canvas * const canvas);
    void clearAll();

    RenderWidget *getRenderWidget();
    TimelineLayout* getTimelineLayout() {
        return mTimelineLayout;
    }
private:
    void setResolutionFractionText(QString text);

    void interruptPreview();

    void setLocalPivot(const bool bT);

    void setTimelineMode();
    void setRenderMode();
    void playPreview();
    void renderPreview();
    void pausePreview();
    void resumePreview();
private:
    Document& mDocument;

    TimelineLayout* mTimelineLayout;

    QWidget* mCentralWidget = nullptr;

    QToolBar *mToolBar;

    QVBoxLayout *mMainLayout;

    QLabel *mControlButtonsWidget;

    QComboBox *mResolutionComboBox;

    ActionButton *mPlayButton;
    ActionButton *mStopButton;
    ActionButton *mLocalPivot;

    QAction *mTimelineAction;
    QAction *mRenderAction;

    QList<TimelineWidget*> mBoxesListKeysViewWidgets;
    RenderWidget *mRenderWidget;
    AnimationDockWidget *mAnimationDockWidget;

    MainWindow *mMainWindow;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
