#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include "smartPointers/sharedpointerdefs.h"
class ScrollArea;
class Canvas;
class RenderInstanceWidget;
class RenderInstanceSettings;

class RenderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = nullptr);

    void createNewRenderInstanceWidgetForCanvas(Canvas *canvas);
    void removeRenderInstanceWidget(RenderInstanceWidget *wid);
    void setRenderedFrame(const int &frame);
private:
    QVBoxLayout *mMainLayout;
    QProgressBar *mRenderProgressBar;
    QHBoxLayout *mButtonsLayout;
    QLabel *mCurrentRenderLabel;
    QPushButton *mStartRenderButton;
    QPushButton *mPauseRenderButton;
    QPushButton *mStopRenderButton;
    QWidget *mContWidget;
    QVBoxLayout *mContLayout;
    ScrollArea *mScrollArea;
    QList<RenderInstanceWidget*> mRenderInstanceWidgets;
    RenderInstanceSettings *mCurrentRenderedSettings = nullptr;
    QList<RenderInstanceWidget*> mAwaitingSettings;

    void render(RenderInstanceSettings *settings);
signals:
    void renderFromSettings(RenderInstanceSettings *);
public slots:
    void leaveOnlyInterruptionButtonsEnabled();
    void leaveOnlyStartRenderButtonEnabled();
    void disableButtons();
    void enableButtons();

    void render();
    void stopRendering();
    void clearAwaitingRender();
    void sendNextForRender();
};

#endif // RENDERWIDGET_H
