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
#include <unordered_map>

#include "boundingboxrendercontainer.h"

class KeysView;

class UndoRedo;

class Canvas;

class UndoRedoStack;

class MovablePoint;

class PathPoint;

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
    TYPE_PARTICLES
};

class BoxesGroup;

class BoxesListWidget;

class CtrlPoint;

class Edge;

class VectorPath;

class BoxItemWidgetContainer;

class EffectsSettingsWidget;

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
    BoundingBox(BoxesGroup *parent, BoundingBoxType type);
    BoundingBox(BoundingBoxType type);
    virtual ~BoundingBox();

    QMatrix getUpdateTransform() { return mUpdateTransform; }

    virtual BoundingBox *createLink(BoxesGroup *parent);
    virtual BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    virtual void setFont(QFont) {}
    virtual void setSelectedFontSize(qreal) {}
    virtual void setSelectedFontFamilyAndStyle(QString,
                                       QString) {}

    virtual QPointF getRelCenterPosition() { return mRelBoundingRect.center(); }
    virtual void centerPivotPosition(bool finish = false) {
        mTransformAnimator.setPivotWithoutChangingTransformation(
                    getRelCenterPosition(), finish);
    }
    virtual bool isContainedIn(QRectF absRect);

    virtual void drawPixmap(QPainter *p);
    virtual void drawPreviewPixmap(QPainter *p);
    virtual void renderFinal(QPainter *p);

    virtual void draw(QPainter *) {}

    virtual void drawSelected(QPainter *p,
                              const CanvasMode &) {
        if(isVisibleAndInVisibleDurationRect()) {
            p->save();
            drawBoundingRect(p);
            p->restore();
        }
    }


    void applyTransformation(TransformAnimator *transAnimator);

    void rotateBy(qreal rot, QPointF absOrigin);

    QPointF getAbsolutePos();

    virtual void updateCombinedTransform();
    void moveByRel(QPointF trans);

    void startTransform();
    void finishTransform();


    virtual bool relPointInsidePath(QPointF) { return false; }
    bool absPointInsidePath(QPointF absPos);
    virtual MovablePoint *getPointAt(const QPointF &,
                                     const CanvasMode &) { return NULL; }

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

    virtual PaintSettings *getFillSettings();
    virtual StrokeSettings *getStrokeSettings();

    void setPivotAbsPos(QPointF absPos,
                        bool saveUndoRedo = true,
                        bool pivotChanged = true);

    void setPivotRelPos(QPointF relPos,
                        bool saveUndoRedo = true,
                        bool pivotChanged = true);

    void cancelTransform();
    void scale(qreal scaleXBy, qreal scaleYBy);

    virtual int prp_saveToSql(QSqlQuery *query, const int &parentId);

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

    virtual void setFillGradient(Gradient* gradient, bool finish) {
        Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setStrokeGradient(Gradient* gradient, bool finish) {
        Q_UNUSED(gradient); Q_UNUSED(finish); }
    virtual void setFillFlatColor(Color color, bool finish) {
        Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setStrokeFlatColor(Color color, bool finish) {
        Q_UNUSED(color); Q_UNUSED(finish); }
    virtual void setFillPaintType(PaintType paintType,
                                  Color color,
                                  Gradient* gradient) {
        Q_UNUSED(paintType); Q_UNUSED(color); Q_UNUSED(gradient); }
    virtual void setStrokePaintType(PaintType paintType,
                                    Color color,
                                    Gradient* gradient) {
        Q_UNUSED(paintType); Q_UNUSED(color); Q_UNUSED(gradient); }
    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        Q_UNUSED(capStyle); }
    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        Q_UNUSED(joinStyle); }
    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        Q_UNUSED(strokeWidth); Q_UNUSED(finish); }

    virtual void startSelectedStrokeWidthTransform() {}
    virtual void startSelectedStrokeColorTransform() {}
    virtual void startSelectedFillColorTransform() {}

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
    virtual void prp_loadFromSql(const int &boundingBoxId);

    virtual void updatePixmaps();
    void updatePrettyPixmap();

    void saveOldPixmap();

    void saveUglyPaintTransform();
    void drawAsBoundingRect(QPainter *p, QPainterPath path);
    virtual void setUpdateVars();
    void redoUpdate();
    bool shouldRedoUpdate();
    void setRedoUpdateToFalse();

    virtual void afterSuccessfulUpdate() {}

    void updateRelativeTransformTmp();

    virtual void updateAllUglyPixmap();
    QPointF mapAbsPosToRel(QPointF absPos);
    void addEffect(PixmapEffect *effect);
    void removeEffect(PixmapEffect *effect);
    void setAwaitUpdateScheduled(bool bT);

    void setCompositionMode(QPainter::CompositionMode compositionMode);

    virtual void updateEffectsMargin();

    virtual void scheduleEffectsMarginUpdate();
    void updateEffectsMarginIfNeeded();
    virtual QMatrix getCombinedFinalRenderTransform();
    virtual void updateAllBoxes();
    void selectionChangeTriggered(bool shiftPressed);

    bool isAnimated() { return prp_isDescendantRecording(); }
    virtual void updateRelBoundingRect();
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

    virtual QImage getAllUglyPixmapProvidedTransform(
                        const qreal &effectsMargin,
                        const QMatrix &allUglyTransform,
                        QRectF *allUglyBoundingRectP);
    virtual QImage getPrettyPixmapProvidedTransform(
            const QMatrix &transform,
            QRectF *pixBoundingRectClippedToViewP);

    virtual Canvas *getParentCanvas();


    void duplicateTransformAnimatorFrom(TransformAnimator *source);
    virtual void preUpdatePixmapsUpdates();
    void scheduleCenterPivot();

    SWT_Type SWT_getType() { return SWT_BoundingBox; }

    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &);

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
    void moveByAbs(QPointF trans);
    virtual void makeDuplicate(BoundingBox *targetBox);
    BoundingBox *createDuplicate(BoxesGroup *parent);
    virtual BoundingBox *createNewDuplicate(BoxesGroup *) = 0;
    BoundingBox *createDuplicate() {
        return createDuplicate(mParent);
    }

    virtual void drawHovered(QPainter *p) {
        p->save();
        p->setTransform(QTransform(mCombinedTransformMatrix), true);
        QPen pen = QPen(Qt::red, 1.);
        pen.setCosmetic(true);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        p->drawPath(mRelBoundingRectPath);
        p->restore();
    }

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

    void applyEffects(QImage *im,
                      qreal scale = 1.);
    virtual void clearCache();
    virtual QMatrix getCombinedTransform() const;
    virtual void drawUpdatePixmap(QPainter *p);

    virtual void processUpdate();
    virtual void afterUpdate();
    virtual void beforeUpdate();
    virtual void updateCombinedTransformTmp();
    void updateRelativeTransformAfterFrameChange();
    void setNoCache(const bool &bT);
    QPainter::CompositionMode getCompositionMode();
    void drawUpdatePixmapForEffect(QPainter *p);
    QRectF getUpdateRenderRect();
    QMatrix getUpdatePaintTransform();
    bool isParticleBox();
    virtual DurationRectangleMovable *getRectangleMovableAtPos(qreal relX,
                                                       int minViewedFrame,
                                                       qreal pixelsPerFrame) {
        Q_UNUSED(relX);
        Q_UNUSED(minViewedFrame);
        Q_UNUSED(pixelsPerFrame);
        return NULL;
    }
    void getKeysInRect(const QRectF &selectionRect,
                       const qreal &pixelsPerFrame,
                       QList<Key *> *keysList);
    Key *getKeyAtPos(const qreal &relX,
                     const int &minViewedFrame,
                     const qreal &pixelsPerFrame);
    int anim_getFrameShift() const;
    int anim_getParentFrameShift() const;

    void setDurationRectangle(DurationRectangle *durationRect);

    bool isInVisibleDurationRect();
    bool isVisibleAndInVisibleDurationRect();
    bool isUsedAsTarget();
    void incUsedAsTarget();
    void decUsedAsTarget();
    bool shouldUpdateAndDraw();
    void ca_childAnimatorIsRecordingChanged();
protected:
    DurationRectangle *mDurationRectangle = NULL;
    SingleWidgetAbstraction *mSelectedAbstraction = NULL;
    SingleWidgetAbstraction *mTimelineAbstraction = NULL;

    bool mCenterPivotScheduled = false;
    QPointF mPreviewDrawPos;
    QRectF mRelBoundingRect;

    bool mEffectsMarginUpdateNeeded = false;
    qreal mEffectsMargin = 2.;

    bool mAwaitUpdateScheduled = false;

    virtual void updateAfterCombinedTransformationChanged() {}

    std::unordered_map<int, BoundingBoxRenderContainer*> mRenderContainers;

    BoundingBoxRenderContainer *getRenderContainerAtFrame(
                                    const int &frame);

    BoundingBoxRenderContainer *mUpdateRenderContainer =
            new BoundingBoxRenderContainer();

    BoundingBoxRenderContainer *mOldRenderContainer = mUpdateRenderContainer;

    QMatrix mRelativeTransformMatrix;

    int mCurrentAbsFrame = 0;
    int mCurrentRelFrame = 0;
    bool mNoCache = false;
    int mUpdateRelFrame = 0;
    QRectF mUpdateRelBoundingRect;
    QMatrix mUpdateTransform;
    bool mUpdateDrawOnParentBox = true;
    bool mUpdateReplaceCache;

    bool mRedoUpdate = false;
    bool mAwaitingUpdate = false;

    QPainterPath mRelBoundingRectPath;

    int mUsedAsTargetCount = 0;

    bool mScheduledForRemove = false;

    void setType(const BoundingBoxType &type) { mType = type; }
    BoundingBoxType mType;
    BoxesGroup *mParent = NULL;

    EffectAnimators mEffectsAnimators;

    TransformAnimator mTransformAnimator;

    QMatrix mCombinedTransformMatrix;
    int mZListIndex = 0;
    bool mPivotChanged = false;

    QPainter::CompositionMode mCompositionMode = QPainter::CompositionMode_SourceOver;

    bool mVisible = true;
    bool mLocked = false;

    bool mAnimated = false;

    QImage mRenderPixmap;
signals:
    void scheduledUpdate();
    void scheduleAwaitUpdateAllLinkBoxes();
public slots:
    virtual void scheduleUpdate();
};


#endif // CHILDPARENT_H
