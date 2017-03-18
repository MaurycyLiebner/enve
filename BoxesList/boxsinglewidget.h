#ifndef BOXSINGLEWIDGET_H
#define BOXSINGLEWIDGET_H

#include "OptimalScrollArea/singlewidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMimeData>
#include <QComboBox>
#include "boxeslistactionbutton.h"
#include "boxtargetwidget.h"
class QrealAnimatorValueSlider;
class QrealKey;
class BoundingBox;
class DurationRectangleMovable;

const int BOX_HEIGHT = 20;

class BoxSingleWidget : public SingleWidget
{
    Q_OBJECT
public:
    explicit BoxSingleWidget(ScrollWidgetVisiblePart *parent = 0);

    void setTargetAbstraction(SingleWidgetAbstraction *abs);

    static QPixmap *VISIBLE_PIXMAP;
    static QPixmap *INVISIBLE_PIXMAP;
    static QPixmap *HIDE_CHILDREN;
    static QPixmap *SHOW_CHILDREN;
    static QPixmap *LOCKED_PIXMAP;
    static QPixmap *UNLOCKED_PIXMAP;
    static QPixmap *ANIMATOR_CHILDREN_VISIBLE;
    static QPixmap *ANIMATOR_CHILDREN_HIDDEN;
    static QPixmap *ANIMATOR_RECORDING;
    static QPixmap *ANIMATOR_NOT_RECORDING;
    static bool mStaticPixmapsLoaded;
    static void loadStaticPixmaps();

    void rename();
    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int containerTop,
                  int minViewedFrame, int maxViewedFrame);
    QrealKey *getKeyAtPos(const int &pressX,
                          const qreal &pixelsPerFrame,
                          const int &minViewedFrame);
    void getKeysInRect(QRectF selectionRect,
                       qreal pixelsPerFrame,
                       QList<QrealKey *> *listKeys);
    DurationRectangleMovable *getRectangleMovableAtPos(
                                const int &pressX,
                                const qreal &pixelsPerFrame,
                                const int &minViewedFrame);
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *);

    bool mCompositionModeVisible = false;
    void updateCompositionBoxVisible();
signals:

private slots:
    void switchContentVisibleAction();
    void switchRecordingAction();
    void switchBoxLockedAction();

    void switchBoxVisibleAction();
    void openColorSettingsDialog();
    void setCompositionMode(const int &id);
private:
    BoxesListActionButton *mRecordButton;
    BoxesListActionButton *mContentButton;
    BoxesListActionButton *mVisibleButton;
    BoxesListActionButton *mLockedButton;
    BoxesListActionButton *mColorButton;
    BoxTargetWidget *mBoxTargetWidget;

    QPoint mDragStartPos;
    QWidget *mFillWidget;
    QHBoxLayout *mMainLayout;
    QrealAnimatorValueSlider *mValueSlider;
    QComboBox *mCompositionModeCombo;
};

#endif // BOXSINGLEWIDGET_H
