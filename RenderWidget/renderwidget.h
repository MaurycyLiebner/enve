#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
class ScrollArea;
class Canvas;
class RenderInstanceWidget;
class RenderInstanceSettings;

class RenderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = 0);

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
    RenderInstanceSettings *mCurrentRenderedSettings = NULL;
signals:
    void renderFromSettings(RenderInstanceSettings *);
public slots:
    void render();
    void stopRendering();
};

#endif // RENDERWIDGET_H
