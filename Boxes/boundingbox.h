#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "transformable.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
#include "Animators/transformanimator.h"

#include "Animators/animatorscollection.h"
#include "PixmapEffects/pixmapeffect.h"

#include "Animators/effectanimators.h"

#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"

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
    TYPE_CANVAS,
    TYPE_INTERNAL_LINK,
    TYPE_EXTERNAL_LINK,
    TYPE_PARTICLES
};

class BoxesGroup;

class BoxesListWidget;

class CtrlPoint;

class Edge;

class VectorPath;

class BoxItemWidgetContainer;

class EffectsSettingsWidget;

class BoundingBox : public QObject,
        public Transformable,
        public SingleWidgetTarget
{
    Q_OBJECT
public:
    BoundingBox(BoxesGroup *parent, BoundingBoxType type);
    BoundingBox(BoundingBoxType type);
    virtual ~BoundingBox();

    virtual BoundingBox *createLink(BoxesGroup *parent);
    virtual BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    virtual void setFont(QFont) {}
    virtual void setFontSize(qreal) {}
    virtual void setFontFamilyAndStyle(QString,
                                       QString) {}

    virtual QPointF getRelCenterPosition() { return mRelBoundingRect.center(); }
    virtual void centerPivotPosition(bool finish = false) {
        mTransformAnimator.setPivotWithoutChangingTransformation(
                    getRelCenterPosition(), finish);
    }
    virtual bool isContainedIn(QRectF absRect);
    virtual QRectF getPixBoundingRect();

    void drawPixmap(QPainter *p);
    virtual void drawPreviewPixmap(QPainter *p);
    virtual void renderFinal(QPainter *p);

    virtual void draw(QPainter *) {}
    virtual void drawForPreview(QPainter *p) { draw(p); }

    virtual void drawSelected(QPainter *p, CanvasMode) {
        if(mVisible) {
            p->save();
            drawBoundingRect(p);
            p->restore();
        }
    }


    QMatrix getCombinedTransform() const;

    void applyTransformation(TransformAnimator *transAnimator);

    void rotateBy(qreal rot, QPointF absOrigin);

    QPointF getAbsolutePos();

    virtual void updateCombinedTransform();
    void moveBy(QPointF trans);

    void startTransform();
    void finishTransform();


    virtual bool relPointInsidePath(QPointF) { return false; }
    bool absPointInsidePath(QPointF absPos);
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
    void saveTransformPivotAbsPos(QPointF absPivot);

    void setName(QString name);
    QString getName();

    void hide();
    void show();
    bool isVisible();
    void setVisibile(bool visible, bool saveUndoRedo = true);
    void switchVisible();

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

    virtual void setFillGradient(Gradient* gradient, bool finish) {
        Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setStrokeGradient(Gradient* gradient, bool finish) {
        Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setFillFlatColor(Color color, bool finish) {
        Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setStrokeFlatColor(Color color, bool finish) {
        Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient) {
        Q_UNUSED(paintType); Q_UNUSED(color); Q_UNUSED(gradient); }
    virtual void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient) {
        Q_UNUSED(paintType); Q_UNUSED(color); Q_UNUSED(gradient); }
    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        Q_UNUSED(capStyle); }
    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        Q_UNUSED(joinStyle); }
    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        Q_UNUSED(strokeWidth); Q_UNUSED(finish); }

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
    bool isInternalLink();
    bool isExternalLink();
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

    virtual void updateEffectsMargin();

    virtual void scheduleEffectsMarginUpdate();
    void updateEffectsMarginIfNeeded();
    virtual QMatrix getCombinedFinalRenderTransform();
    virtual void updateAllBoxes();
    void selectionChangeTriggered(bool shiftPressed);
    QrealAnimator *getAnimatorsCollection();

    bool isAnimated() { return mAnimatorsCollection.isDescendantRecording(); }
    virtual void updateBoundingRect();
    void updatePixBoundingRectClippedToView();
    virtual const QPainterPath &getRelBoundingRectPath();
    virtual QMatrix getRelativeTransform() const;
    QPointF mapRelativeToAbsolute(QPointF relPos) const;

    QRectF getRelBoundingRect() const {
        return mRelBoundingRect;
    }

    virtual void applyCurrentTransformation() {}

    virtual qreal getEffectsMargin() {
        return mEffectsMargin;
    }

    void setBaseTransformation(const QMatrix &matrix);
    bool hasBaseTransformation();
    virtual QPixmap renderPreviewProvidedTransform(
                        const qreal &effectsMargin,
                        const qreal &resolutionScale,
                        const QMatrix &renderTransform,
                        QPointF *drawPos);
    virtual QPixmap getAllUglyPixmapProvidedTransform(
                        const qreal &effectsMargin,
                        const QMatrix &allUglyTransform,
                        QRectF *allUglyBoundingRectP);
    virtual QPixmap getPrettyPixmapProvidedTransform(
            const QMatrix &transform,
            QRectF *pixBoundingRectClippedToViewP);

    virtual Canvas *getParentCanvas();


    void duplicateTransformAnimatorFrom(TransformAnimator *source);
    virtual void preUpdatePixmapsUpdates();
    void updatePreviewPixmap();
    void scheduleCenterPivot();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);
    SWT_Type SWT_getType() { return SWT_BoundingBox; }

    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_satisfiesRule(const SWT_RulesCollection &rules,
                           const bool &parentSatisfies);

    void setUpdateDisabled(const bool &bT) {
        mUpdateDisabled = bT;
    }

    bool SWT_visibleOnlyIfParentDescendant() {
        return false;
    }

    void SWT_addToContextMenu(QMenu *menu);
    bool SWT_handleContextMenuActionSelected(QAction *selectedAction);
protected:
    bool mUpdateDisabled = false;

    SingleWidgetAbstraction *mSelectedAbstraction = NULL;
    SingleWidgetAbstraction *mTimelineAbstraction = NULL;

    bool mCenterPivotScheduled = false;
    QPointF mPreviewDrawPos;
    QRectF mRelBoundingRect;

    bool mHighQualityPaint = false;
    bool mEffectsMarginUpdateNeeded = false;
    qreal mEffectsMargin = 2.;

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
    QPainterPath mRelBoundingRectPath;
    QPainterPath mMappedBoundingRectPath;

    bool mScheduledForRemove = false;

    void setType(const BoundingBoxType &type) { mType = type; }
    BoundingBoxType mType;
    BoxesGroup *mParent = NULL;

    AnimatorsCollection mAnimatorsCollection;
    EffectAnimators mEffectsAnimators;

    TransformAnimator mTransformAnimator;

    QMatrix mCombinedTransformMatrix;
    int mZListIndex = 0;
    bool mPivotChanged = false;

    QPainter::CompositionMode mCompositionMode = QPainter::CompositionMode_SourceOver;

    bool mVisible = true;
    bool mLocked = false;

    QString mName = "";
    QList<QrealAnimator*> mActiveAnimators;

    bool mAnimated = false;

    QPixmap mRenderPixmap;
signals:
    void scheduleAwaitUpdateAllLinkBoxes();
    void addActiveAnimatorSignal(QrealAnimator*);
    void removeActiveAnimatorSignal(QrealAnimator*);
};


#endif // CHILDPARENT_H
