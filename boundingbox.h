#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "transformable.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
#include "transformanimator.h"

#include "animatorscollection.h"
#include "pixmapeffect.h"

class KeysView;

class UndoRedo;

class Canvas;

class UndoRedoStack;

class MovablePoint;

class PathPoint;

enum CanvasMode : short;

enum BoundingBoxType {
    TYPE_VECTOR_PATH,
    TYPE_CIRCLE,
    TYPE_IMAGE,
    TYPE_RECTANGLE,
    TYPE_TEXT,
    TYPE_GROUP,
    TYPE_CANVAS
};

class BoxesGroup;

class BoxesListWidget;

class CtrlPoint;

class Edge;

class VectorPath;

class BoxItemWidgetContainer;

class BoundingBox : public QObject, public Transformable
{
    Q_OBJECT
public:
    BoundingBox(BoxesGroup *parent, BoundingBoxType type);
    BoundingBox(BoundingBoxType type);

    virtual void setFont(QFont) {}
    virtual void setFontSize(qreal) {}
    virtual void setFontFamilyAndStyle(QString,
                                       QString) {}

    virtual void centerPivotPosition(bool finish = false) { Q_UNUSED(finish); }
    virtual bool isContainedIn(QRectF absRect);
    virtual QRectF getPixBoundingRect();

    void drawPixmap(QPainter *p);
    virtual void render(QPainter *p);
    virtual void renderFinal(QPainter *p);
    virtual void draw(QPainter *) {}
    virtual void drawSelected(QPainter *, CanvasMode) {}

    QMatrix getCombinedTransform() const;

    void applyTransformation(TransformAnimator *transAnimator);

    void rotateBy(qreal rot, QPointF absOrigin);

    QPointF getAbsolutePos();

    void updateCombinedTransform();
    void moveBy(QPointF trans);

    void startTransform();
    void finishTransform();


    virtual bool absPointInsidePath(QPointF) { return false; }
    virtual MovablePoint *getPointAt(QPointF, CanvasMode) { return NULL; }

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();

    void setZListIndex(int z, bool saveUndoRedo = true);

    virtual void selectAndAddContainedPointsToList
                            (QRectF,QList<MovablePoint*> *) {}

    QPointF getPivotAbsPos();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRect(QPainter *p);
    void setParent(BoxesGroup *parent, bool saveUndoRedo = true);
    BoxesGroup *getParent();

    bool isGroup();
    virtual BoundingBox *getPathAtFromAllAncestors(QPointF absPos);

    virtual const PaintSettings *getFillSettings();
    virtual const StrokeSettings *getStrokeSettings();

    virtual qreal getCurrentCanvasScale();

    void setPivotAbsPos(QPointF absPos, bool saveUndoRedo = true, bool pivotChanged = true);

    void setPivotRelPos(QPointF relPos, bool saveUndoRedo = true, bool pivotChanged = true);

    void cancelTransform();
    void scale(qreal scaleXBy, qreal scaleYBy);

    virtual int saveToSql(QSqlQuery *query, int parentId);

    virtual PathPoint *createNewPointOnLineNear(QPointF, bool) { return NULL; }
    bool isVectorPath();
    void saveTransformPivot(QPointF absPivot);

    void setName(QString name);
    QString getName();

    void hide();
    void show();
    bool isVisible();
    void setVisibile(bool visible, bool saveUndoRedo = true);

    void setChildrenListItemsVisible(bool bt);
    void lock();
    void unlock();
    void setLocked(bool bt);
    bool isLocked();
    bool isVisibleAndUnlocked();
    void rotateBy(qreal rot);
    void scale(qreal scaleBy);

    void rotateRelativeToSavedPivot(qreal rot);
    void scaleRelativeToSavedPivot(qreal scaleBy);

    virtual void startPosTransform();
    virtual void startRotTransform();
    virtual void startScaleTransform();
    virtual void updateAfterFrameChanged(int currentFrame);
    virtual QMatrix getCombinedRenderTransform();

    virtual void startAllPointsTransform() {}
    virtual void finishAllPointsTransform() {}

    void addActiveAnimator(QrealAnimator *animator);
    void removeActiveAnimator(QrealAnimator *animator);

    virtual void setFillGradient(Gradient* gradient, bool finish) { Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setStrokeGradient(Gradient* gradient, bool finish) { Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setFillFlatColor(Color color, bool finish) { Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setStrokeFlatColor(Color color, bool finish) { Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient) { Q_UNUSED(paintType); Q_UNUSED(color); Q_UNUSED(gradient); }
    virtual void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient) { Q_UNUSED(paintType); Q_UNUSED(color); Q_UNUSED(gradient); }
    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) { Q_UNUSED(capStyle); }
    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) { Q_UNUSED(joinStyle); }
    virtual void setStrokeWidth(qreal strokeWidth, bool finish) { Q_UNUSED(strokeWidth); Q_UNUSED(finish); }

    virtual void startStrokeWidthTransform() {}
    virtual void startStrokeColorTransform() {}
    virtual void startFillColorTransform() {}

    virtual Edge *getEgde(QPointF absPos) {
        Q_UNUSED(absPos);
        return NULL;
    }
    void setAbsolutePos(QPointF pos, bool saveUndoRedo);
    void setRelativePos(QPointF relPos, bool saveUndoRedo);

    virtual void showContextMenu(QPoint globalPos) { Q_UNUSED(globalPos); }

    virtual void drawKeys(QPainter *p, qreal pixelsPerFrame,
                          qreal drawY, int startFrame, int endFrame);
    void scaleRelativeToSavedPivot(qreal scaleXBy, qreal scaleYBy);
    void resetScale();
    void resetTranslation();
    void resetRotation();
    bool isCircle();
    bool isRect();
    bool isText();
    TransformAnimator *getTransformAnimator();
    void disablePivotAutoAdjust();
    void enablePivotAutoAdjust();
    void copyTransformationTo(BoundingBox *box);

    virtual VectorPath *objectToPath() { return NULL; }
    virtual void loadFromSql(int boundingBoxId);

    virtual void updatePixmaps();
    void updatePrettyPixmap();
    void setAwaitingUpdate(bool bT);
    virtual void awaitUpdate();
    QRectF getBoundingRectClippedToView();
    void saveOldPixmap();

    void saveUglyPaintTransform();
    void drawAsBoundingRect(QPainter *p, QPainterPath path);
    virtual void updateUpdateTransform();
    void updateUglyPaintTransform();
    void redoUpdate();
    bool shouldRedoUpdate();
    void setRedoUpdateToFalse();

    virtual void afterSuccessfulUpdate() {}

    void updateRelativeTransform();
    void updateAllUglyPixmap();
    QPointF mapAbsPosToRel(QPointF absPos);
    void addEffect(PixmapEffect *effect);
    void removeEffect(PixmapEffect *effect);
    void scheduleAwaitUpdate();
    void setAwaitUpdateScheduled(bool bT);

    void setCompositionMode(QPainter::CompositionMode compositionMode);

    void updateEffectsMargin();

    void scheduleEffectsMarginUpdate();
    void updateEffectsMarginIfNeeded();
    virtual QMatrix getCombinedFinalRenderTransform();
    virtual void updateAllBoxes();
    void selectionChangeTriggered(bool shiftPressed);
    void addAllAnimatorsToBoxItemWidgetContainer(BoxItemWidgetContainer *container);
    QrealAnimator *getAnimatorsCollection();

    bool isAnimated() { return mAnimated; }
    void setAnimated(bool bT);
    virtual void updateBoundingRect() {}
    void updatePixBoundingRectClippedToView();
    const QPainterPath &getBoundingRectPath();
    QMatrix getRelativeTransform() const;
    QPointF mapRelativeToAbsolute(QPointF relPos) const;

    virtual void applyCurrentTransformation() {}
protected:
    QRectF mRelBoundingRect;

    bool mHighQualityPaint = false;
    bool mEffectsMarginUpdateNeeded = false;
    qreal mEffectsMargin = 0.;

    bool mAwaitUpdateScheduled = false;

    virtual void updateAfterCombinedTransformationChanged() {}
    QPixmap applyEffects(const QPixmap &pixmap,
                         bool highQuality,
                         qreal scale = 1.);

    QMatrix mAllUglyTransform;
    QMatrix mAllUglyPaintTransform;
    QRectF mAllUglyBoundingRect;

    QMatrix mOldAllUglyTransform;
    QMatrix mOldAllUglyPaintTransform;
    QRectF mOldAllUglyBoundingRect;
    QPixmap mOldAllUglyPixmap;

    QMatrix mRelativeTransformMatrix;

    QMatrix mUpdateCanvasTransform;
    QMatrix mUpdateTransform;
    QMatrix mOldTransform;
    QPixmap mNewPixmap;
    QPixmap mOldPixmap;
    QPixmap mAllUglyPixmap;
    QRectF mOldPixBoundingRect;

    bool mRedoUpdate = false;
    bool mAwaitingUpdate = false;
    QMatrix mUglyPaintTransform;

    QRectF mPixBoundingRect;
    QRectF mPixBoundingRectClippedToView;
    QPainterPath mBoundingRect;
    QPainterPath mMappedBoundingRect;

    BoxesListWidget *mBoxesList;
    KeysView *mKeysView;

    bool mScheduledForRemove = false;
    BoundingBoxType mType;
    BoxesGroup *mParent = NULL;

    AnimatorsCollection mAnimatorsCollection;
    ComplexAnimator mEffectsAnimators;

    TransformAnimator mTransformAnimator;

    QMatrix mCombinedTransformMatrix;
    int mZListIndex = 0;
    bool mPivotChanged = false;

    QPainter::CompositionMode mCompositionMode = QPainter::CompositionMode_SourceOver;

    bool mVisible = true;
    bool mLocked = false;

    QString mName = "";
    QList<QrealAnimator*> mActiveAnimators;

    QList<PixmapEffect*> mEffects;
    bool mAnimated = false;
signals:
    void addActiveAnimatorSignal(QrealAnimator*);
    void removeActiveAnimatorSignal(QrealAnimator*);
};


#endif // CHILDPARENT_H
