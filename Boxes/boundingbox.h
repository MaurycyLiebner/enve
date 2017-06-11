#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "transformable.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
#include "Animators/transformanimator.h"

#include "PixmapEffects/pixmapeffect.h"

#include "Animators/effectanimators.h"

#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"
#include <unordered_map>

#include "boundingboxrendercontainer.h"
#include "skiaincludes.h"

class Canvas;

class MovablePoint;

class PathPoint;
class PathEffect;
class PathAnimator;
class DurationRectangleMovable;

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
    TYPE_PARTICLES,
    TYPE_DRAFT_CANVAS
};

class BoxesGroup;

class VectorPathEdge;

class VectorPath;

class DurationRectangle;

class BoundingBoxMimeData : public QMimeData {
    Q_OBJECT
public:
    BoundingBoxMimeData(BoundingBox *target) : QMimeData() {
        mBoundingBox = target;
    }

    BoundingBox *getBoundingBox() {
        return mBoundingBox;
    }

    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "boundingbox") return true;
        return false;
    }
private:
    BoundingBox *mBoundingBox;
};

class BoundingBox :
        public ComplexAnimator,
        public Transformable {
    Q_OBJECT
public:
    BoundingBox(BoxesGroup *parent,
                const BoundingBoxType &type);
    BoundingBox(const BoundingBoxType &type);
    virtual ~BoundingBox();

    const QMatrix &getUpdateTransform() { return mUpdateTransform; }

    virtual BoundingBox *createLink(BoxesGroup *parent);
    virtual BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    virtual void setFont(QFont) {}
    virtual void setSelectedFontSize(qreal) {}
    virtual void setSelectedFontFamilyAndStyle(QString, QString) {}

    virtual QPointF getRelCenterPosition() {
        return mRelBoundingRect.center();
    }

    virtual void centerPivotPosition(bool finish = false) {
        mTransformAnimator->setPivotWithoutChangingTransformation(
                    getRelCenterPosition(), finish);
    }
    virtual bool isContainedIn(const QRectF &absRect);

    virtual void drawPixmapSk(SkCanvas *canvas);

    virtual void drawSk(SkCanvas *) {}

    virtual void drawSelectedSk(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const qreal &invScale);


    void applyTransformation(BoxTransformAnimator *transAnimator);

    void rotateBy(const qreal &rot, QPointF absOrigin);

    QPointF getAbsolutePos();

    virtual void updateCombinedTransform();
    void updateCombinedTransformAfterFrameChange();

    void moveByRel(const QPointF &trans);

    void startTransform();
    void finishTransform();


    virtual bool relPointInsidePath(const QPointF &) { return false; }
    bool absPointInsidePath(const QPointF &absPos);
    virtual MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv);

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();

    void setZListIndex(const int &z,
                       const bool &saveUndoRedo = true);

    virtual void selectAndAddContainedPointsToList(const QRectF &,
                                                   QList<MovablePoint*> *) {}

    QPointF getPivotAbsPos();
    virtual void select();
    void deselect();
    int getZIndex();
    void drawBoundingRectSk(SkCanvas *canvas,
                            const qreal &invScale);

    void setParent(BoxesGroup *parent,
                   const bool &saveUndoRedo = true);
    BoxesGroup *getParent();

    bool isGroup();
    virtual BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    virtual PaintSettings *getFillSettings();
    virtual StrokeSettings *getStrokeSettings();

    void setPivotAbsPos(const QPointF &absPos,
                        const bool &saveUndoRedo = true,
                        const bool &pivotChanged = true);

    void setPivotRelPos(const QPointF &relPos,
                        const bool &saveUndoRedo = true,
                        const bool &pivotChanged = true);

    void cancelTransform();
    void scale(const qreal &scaleXBy,
               const qreal &scaleYBy);

    virtual int saveToSql(QSqlQuery *query, const int &parentId);

    virtual PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(adjust);
        Q_UNUSED(canvasScaleInv);
        return NULL;
    }
    bool isVectorPath();
    void saveTransformPivotAbsPos(const QPointF &absPivot);

    void setName(const QString &name);
    QString getName();

    void hide();
    void show();
    bool isVisible();
    void setVisibile(const bool &visible,
                     const bool &saveUndoRedo = true);
    void switchVisible();

    void lock();
    void unlock();
    void setLocked(const bool &bt);
    bool isLocked();
    bool isVisibleAndUnlocked();
    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleBy);

    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &scaleBy);

    virtual void startPosTransform();
    virtual void startRotTransform();
    virtual void startScaleTransform();
    virtual void updateAfterFrameChanged(const int &currentFrame);
    virtual QMatrix getCombinedRenderTransform();

    virtual void startAllPointsTransform() {}
    virtual void finishAllPointsTransform() {}

    virtual void setFillGradient(Gradient* gradient, bool finish) {
        Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setStrokeGradient(Gradient* gradient, bool finish) {
        Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setFillFlatColor(Color color, bool finish) {
        Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setStrokeFlatColor(Color color, bool finish) {
        Q_UNUSED(color); Q_UNUSED(finish); }

    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        Q_UNUSED(capStyle); }
    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        Q_UNUSED(joinStyle); }
    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        Q_UNUSED(strokeWidth); Q_UNUSED(finish); }

    virtual void startSelectedStrokeWidthTransform() {}
    virtual void startSelectedStrokeColorTransform() {}
    virtual void startSelectedFillColorTransform() {}

    virtual VectorPathEdge *getEgde(const QPointF &absPos,
                                    const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(canvasScaleInv);
        return NULL;
    }
    void setAbsolutePos(const QPointF &pos,
                        const bool &saveUndoRedo = false);
    void setRelativePos(const QPointF &relPos,
                        const bool &saveUndoRedo = false);

    virtual void showContextMenu(QPoint globalPos) { Q_UNUSED(globalPos); }


    void scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                   const qreal &scaleYBy);
    void resetScale();
    void resetTranslation();
    void resetRotation();
    bool isCircle();
    bool isRect();
    bool isText();
    bool isInternalLink();
    bool isExternalLink();
    BoxTransformAnimator *getTransformAnimator();
    void disablePivotAutoAdjust();
    void enablePivotAutoAdjust();

    virtual VectorPath *objectToPath() { return NULL; }
    virtual VectorPath *strokeToPath() { return NULL; }
    virtual void prp_loadFromSql(const int &boundingBoxId);

    virtual void updatePixmaps();
    void updatePrettyPixmap();

    void saveOldPixmap();

    void saveUglyPaintTransform();
    void drawAsBoundingRectSk(SkCanvas *canvas,
                              const SkPath &path,
                              const SkScalar &invScale);

    virtual void setUpdateVars();
    void redoUpdate();
    bool shouldRedoUpdate();
    void setRedoUpdateToFalse();

    virtual void afterSuccessfulUpdate() {}

    void updateRelativeTransformTmp();

    virtual void updateAllUglyPixmap();
    virtual QPointF mapAbsPosToRel(const QPointF &absPos);
    void addEffect(PixmapEffect *effect);
    void removeEffect(PixmapEffect *effect);
    void setAwaitUpdateScheduled(bool bT);

    void setBlendModeSk(const SkBlendMode &blendMode);
    const SkBlendMode &getBlendMode() {
        return mBlendModeSk;
    }


    virtual void updateEffectsMargin();

    virtual void scheduleEffectsMarginUpdate();
    void updateEffectsMarginIfNeeded();
    virtual QMatrix getCombinedFinalRenderTransform();
    virtual void updateAllBoxes();
    void selectionChangeTriggered(bool shiftPressed);

    bool isAnimated() { return prp_isDescendantRecording(); }
    virtual void updateRelBoundingRect();
    virtual const SkPath &getRelBoundingRectPath();
    virtual QMatrix getRelativeTransform() const;
    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    QRectF getRelBoundingRect() const {
        return mRelBoundingRect;
    }

    virtual void applyCurrentTransformation() {}

    virtual qreal getEffectsMargin() {
        return mEffectsMargin;
    }

    virtual sk_sp<SkImage> getAllUglyPixmapProvidedTransformSk(
                        const qreal &effectsMargin,
                        const qreal &resolution,
                        const QMatrix &allUglyTransform,
                        SkPoint *drawPosP);

    virtual Canvas *getParentCanvas();


    void duplicateTransformAnimatorFrom(BoxTransformAnimator *source);
    virtual void preUpdatePixmapsUpdates();
    void scheduleCenterPivot();

    bool SWT_isBoundingBox() { return true; }

    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);

    bool SWT_visibleOnlyIfParentDescendant() {
        return false;
    }

    void SWT_addToContextMenu(QMenu *menu);
    bool SWT_handleContextMenuActionSelected(QAction *selectedAction);

    QMimeData *SWT_createMimeData() {
        return new BoundingBoxMimeData(this);
    }

    bool isAncestor(BoxesGroup *box) const;
    bool isAncestor(BoundingBox *box) const;
    void removeFromParent();
    void removeFromSelection();
    void moveByAbs(const QPointF &trans);
    virtual void makeDuplicate(Property *property);
    Property *makeDuplicate();
    BoundingBox *createDuplicate(BoxesGroup *parent);
    virtual BoundingBox *createNewDuplicate(BoxesGroup *) = 0;
    BoundingBox *createDuplicate() {
        return createDuplicate(mParent.data());
    }

    virtual void drawHoveredSk(SkCanvas *canvas,
                               const qreal &invScale) {
        drawHoveredPathSk(canvas, mSkRelBoundingRectPath, invScale);
    }

    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const qreal &invScale);

    virtual void applyPaintSetting(
            const PaintSetting &setting) {
        Q_UNUSED(setting);
    }

    virtual void setFillColorMode(const ColorMode &colorMode) {
        Q_UNUSED(colorMode);
    }
    virtual void setStrokeColorMode(const ColorMode &colorMode) {
        Q_UNUSED(colorMode);
    }
    void switchLocked();

    virtual void removeChildPathAnimator(PathAnimator *path) {
        Q_UNUSED(path);
    }

    void applyEffects(QImage *im, const qreal &scale = 1.);
    void applyEffectsSk(const SkBitmap &im, const qreal &scale = 1.);

    virtual QMatrix getCombinedTransform() const;
    virtual void drawUpdatePixmapSk(SkCanvas *canvas);

    virtual void processUpdate();
    virtual void afterUpdate();
    virtual void beforeUpdate();
    virtual void updateCombinedTransformTmp();
    void updateRelativeTransformAfterFrameChange();
    QPainter::CompositionMode getCompositionMode();
    void drawUpdatePixmapForEffectSk(SkCanvas *canvas);
    QMatrix getUpdatePaintTransform();
    bool isParticleBox();
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                    const qreal &relX,
                                    const int &minViewedFrame,
                                    const qreal &pixelsPerFrame);

    int prp_getFrameShift() const;
    int prp_getParentFrameShift() const;

    void setDurationRectangle(DurationRectangle *durationRect);

    bool isInVisibleDurationRect();
    bool isVisibleAndInVisibleDurationRect();
    bool isUsedAsTarget();
    void incUsedAsTarget();
    void decUsedAsTarget();
    bool shouldUpdate();
    void ca_childAnimatorIsRecordingChanged();

    int getSqlId() {
        return mSqlId;
    }

    void setSqlId(int id) {
        mSqlId = id;
    }
    virtual void clearAllCache();

    void startPivotTransform();
    void finishPivotTransform();
    bool hasDurationRectangle();
    void createDurationRectangle();
    void prp_drawKeys(QPainter *p,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &endFrame);
    virtual void addPathEffect(PathEffect *) {}
protected:
    void updateDrawRenderContainerTransform();
    virtual void scheduleUpdate();

    bool mRenderCacheChangeNeeded = false;
    bool mReplaceCache = false;

    int mSqlId = 0;

    DurationRectangle *mDurationRectangle = NULL;
    SingleWidgetAbstraction *mSelectedAbstraction = NULL;
    SingleWidgetAbstraction *mTimelineAbstraction = NULL;

    bool mCenterPivotScheduled = false;
    QPointF mPreviewDrawPos;
    QRectF mRelBoundingRect;
    SkRect mRelBoundingRectSk;

    bool mEffectsMarginUpdateNeeded = false;
    qreal mEffectsMargin = 2.;

    bool mAwaitUpdateScheduled = false;

    virtual void updateAfterCombinedTransformationChanged() {}
    virtual void updateAfterCombinedTransformationChangedAfterFrameChagne() {}


    RenderContainer mUpdateRenderContainer;
    RenderContainer mDrawRenderContainer;

    int mUpdateRelFrame = 0;
    QRectF mUpdateRelBoundingRect;
    QMatrix mUpdateTransform;
    bool mUpdateDrawOnParentBox = true;
    qreal mUpdateOpacity;

    bool mRedoUpdate = false;
    bool mAwaitingUpdate = false;

    SkPath mSkRelBoundingRectPath;

    int mUsedAsTargetCount = 0;

    bool mScheduledForRemove = false;

    void setType(const BoundingBoxType &type) { mType = type; }
    BoundingBoxType mType;
    QSharedPointer<BoxesGroup> mParent;

    QSharedPointer<EffectAnimators> mEffectsAnimators =
            (new EffectAnimators())->ref<EffectAnimators>();

    QSharedPointer<BoxTransformAnimator> mTransformAnimator =
                (new BoxTransformAnimator(this))->ref<BoxTransformAnimator>();

    int mZListIndex = 0;
    bool mPivotChanged = false;

    QPainter::CompositionMode mCompositionMode =
            QPainter::CompositionMode_SourceOver;
    SkBlendMode mBlendModeSk = SkBlendMode::kSrcOver;

    void getVisibleAbsFrameRange(int *minFrame, int *maxFrame);

    bool mVisible = true;
    bool mLocked = false;

    bool mForceUpdate = false;
signals:
    void replaceChacheSet();
    void scheduledUpdate();
    void scheduleAwaitUpdateAllLinkBoxes();
public slots:
    void updateAfterDurationRectangleShifted(const int &dFrame = 0);
    void updateAfterDurationMinFrameChangedBy(const int &by);
    void updateAfterDurationMaxFrameChangedBy(const int &by);
    virtual void updateAfterDurationRectangleRangeChanged() {}
    void replaceCurrentFrameCache();
    void scheduleSoftUpdate();
    void scheduleHardUpdate();

    void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                             const int &maxFrame);
    void prp_updateInfluenceRangeAfterChanged();
};


#endif // CHILDPARENT_H
