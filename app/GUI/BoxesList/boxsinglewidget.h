#ifndef BOXSINGLEWIDGET_H
#define BOXSINGLEWIDGET_H

#include "OptimalScrollArea/singlewidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMimeData>
#include <QComboBox>
#include "skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"
class QrealAnimatorValueSlider;
class DurationRectangleMovable;
class Key;
class BoxTargetWidget;
class BoxesListActionButton;
class BoolPropertyWidget;
class ComboBoxProperty;

class BoxSingleWidget : public SingleWidget {
    Q_OBJECT
public:
    explicit BoxSingleWidget(ScrollWidgetVisiblePart *parent = nullptr);

    void setTargetAbstraction(SingleWidgetAbstraction *abs);

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
    static bool mStaticPixmapsLoaded;
    static void loadStaticPixmaps();
    static void clearStaticPixmaps();

    void rename();
    void drawKeys(QPainter *p,
                  const qreal &pixelsPerFrame,
                  const int &containerTop,
                  const int &minViewedFrame,
                  const int &maxViewedFrame);
    Key *getKeyAtPos(const int &pressX,
                     const qreal &pixelsPerFrame,
                     const int &minViewedFrame);
    void getKeysInRect(const QRectF &selectionRect,
                       const qreal &pixelsPerFrame,
                       QList<Key *> &listKeys);
    DurationRectangleMovable *getRectangleMovableAtPos(
                        const int &pressX,
                        const qreal &pixelsPerFrame,
                        const int &minViewedFrame);
    void setBlendMode(const SkBlendMode &mode);
    void setSelected(const bool &bT) {
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

    bool mCompositionModeVisible = false;
    void updateCompositionBoxVisible();
    void clearAndHideValueAnimators();
    void updateValueSlidersForQPointFAnimator();
signals:

private slots:
    void clearSelected() {
        setSelected(false);
    }
    void switchContentVisibleAction();
    void switchRecordingAction();
    void switchBoxLockedAction();

    void switchBoxVisibleAction();
    void openColorSettingsDialog();
    void setCompositionMode(const int &id);
private:
    QPoint mDragStartPos;

    BoxesListActionButton *mRecordButton;
    BoxesListActionButton *mContentButton;
    BoxesListActionButton *mVisibleButton;
    BoxesListActionButton *mLockedButton;
    BoxesListActionButton *mColorButton;
    BoxTargetWidget *mBoxTargetWidget;

    qptr<ComboBoxProperty> mLastComboBoxProperty;
    QWidget *mFillWidget;
    BoolPropertyWidget *mCheckBox;
    QHBoxLayout *mMainLayout;
    QrealAnimatorValueSlider *mValueSlider;
    QrealAnimatorValueSlider *mSecondValueSlider;

    QComboBox *mPropertyComboBox;
    QComboBox *mCompositionModeCombo;
};

#endif // BOXSINGLEWIDGET_H
