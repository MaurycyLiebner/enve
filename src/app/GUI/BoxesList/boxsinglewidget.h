#ifndef BOXSINGLEWIDGET_H
#define BOXSINGLEWIDGET_H

#include "OptimalScrollArea/singlewidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMimeData>
#include <QComboBox>
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"
#include "Animators/SmartPath/smartpathanimator.h"
class QrealAnimatorValueSlider;
class DurationRectangleMovable;
class Key;
class BoxTargetWidget;
class BoxesListActionButton;
class PixmapActionButton;
class BoolPropertyWidget;
class ComboBoxProperty;
class ColorAnimator;
class ColorAnimatorButton;
class BoxScroller;

class BoxSingleWidget : public SingleWidget {
public:
    explicit BoxSingleWidget(BoxScroller * const parent);

    void setTargetAbstraction(SWT_Abstraction *abs);

    static QPixmap* VISIBLE_PIXMAP;
    static QPixmap* INVISIBLE_PIXMAP;
    static QPixmap* HIDE_CHILDREN;
    static QPixmap* SHOW_CHILDREN;
    static QPixmap* LOCKED_PIXMAP;
    static QPixmap* UNLOCKED_PIXMAP;
    static QPixmap* ANIMATOR_CHILDREN_VISIBLE;
    static QPixmap* ANIMATOR_CHILDREN_HIDDEN;
    static QPixmap* ANIMATOR_RECORDING;
    static QPixmap* ANIMATOR_NOT_RECORDING;
    static QPixmap* ANIMATOR_DESCENDANT_RECORDING;
    static bool sStaticPixmapsLoaded;
    static void loadStaticPixmaps();
    static void clearStaticPixmaps();

    void drawKeys(QPainter * const p,
                  const qreal pixelsPerFrame,
                  const FrameRange &viewedFrames);
    Key *getKeyAtPos(const int pressX,
                     const qreal pixelsPerFrame,
                     const int minViewedFrame);
    void getKeysInRect(const QRectF &selectionRect,
                       const qreal pixelsPerFrame,
                       QList<Key *> &listKeys);
    DurationRectangleMovable *getRectangleMovableAtPos(
                        const int pressX,
                        const qreal pixelsPerFrame,
                        const int minViewedFrame);

    void setSelected(const bool bT) {
        mSelected = bT;
        update();
    }
    bool isTargetDisabled();
    int getOptimalNameRightX();
protected:
    bool mSelected = false;
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *);

    bool mBlendModeVisible = false;
    bool mPathBlendModeVisible = false;
    bool mFillTypeVisible = false;

    void updatePathCompositionBoxVisible();
    void updateCompositionBoxVisible();
    void updateFillTypeBoxVisible();

    void clearAndHideValueAnimators();
    void updateValueSlidersForQPointFAnimator();
private:
    void clearColorButton();

    void clearSelected() {
        setSelected(false);
    }
    void switchContentVisibleAction();
    void switchRecordingAction();
    void switchBoxLockedAction();

    void switchBoxVisibleAction();
    void setCompositionMode(const int id);
    void setPathCompositionMode(const int id);
    void setFillType(const int id);
    ColorAnimator* getColorTarget() const;

    BoxScroller* const mParent;

    QPoint mDragStartPos;

    PixmapActionButton *mRecordButton;
    PixmapActionButton *mContentButton;
    PixmapActionButton *mVisibleButton;
    PixmapActionButton *mLockedButton;
    ColorAnimatorButton *mColorButton;
    BoxTargetWidget *mBoxTargetWidget;

    qptr<ComboBoxProperty> mLastComboBoxProperty;
    QWidget *mFillWidget;
    BoolPropertyWidget *mCheckBox;
    QHBoxLayout *mMainLayout;
    QrealAnimatorValueSlider *mValueSlider;
    QrealAnimatorValueSlider *mSecondValueSlider;

    QComboBox *mPropertyComboBox;
    QComboBox *mBlendModeCombo;
    QComboBox *mPathBlendModeCombo;
    QComboBox *mFillTypeCombo;
};

#endif // BOXSINGLEWIDGET_H
