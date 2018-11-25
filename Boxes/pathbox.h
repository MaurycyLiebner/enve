#ifndef PATHBOX_H
#define PATHBOX_H
#include "Boxes/boundingbox.h"
#include "Animators/paintsettings.h"
class GradientPoints;
class SkStroke;
class PathEffectAnimators;
typedef QSharedPointer<PathEffectAnimators> PathEffectAnimatorsQSPtr;
class PathEffect;
typedef QSharedPointer<GradientPoints> GradientPointsQSPtr;

struct PathBoxRenderData : public BoundingBoxRenderData {
    PathBoxRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
    }

    SkPath editPath;
    SkPath path;
    SkPath fillPath;
    SkPath outlinePath;
    UpdatePaintSettings paintSettings;
    UpdateStrokeSettings strokeSettings;

    void updateRelBoundingRect() {
        SkPath totalPath;
        totalPath.addPath(fillPath);
        totalPath.addPath(outlinePath);
        relBoundingRect = SkRectToQRectF(totalPath.computeTightBounds());
    }
    QPointF getCenterPosition() {
        return SkRectToQRectF(editPath.getBounds()).center();
    }
private:
    void drawSk(SkCanvas *canvas) {
        canvas->save();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        if(!fillPath.isEmpty()) {
            paintSettings.applyPainterSettingsSk(&paint);

            canvas->drawPath(fillPath, paint);
        }
        if(!outlinePath.isEmpty()) {
            paint.setShader(nullptr);
            strokeSettings.applyPainterSettingsSk(&paint);

            canvas->drawPath(outlinePath, paint);
        }

        canvas->restore();
    }
};

class PathBox : public BoundingBox {
    friend class SelfRef;
public:
    ~PathBox();

    void resetStrokeGradientPointsPos();

    void resetFillGradientPointsPos();

    virtual void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);

    virtual void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);

    virtual void setStrokeWidth(const qreal &strokeWidth, const bool &finish);

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    void startSelectedStrokeWidthTransform();

    void startSelectedStrokeColorTransform();

    void startSelectedFillColorTransform();

    StrokeSettings *getStrokeSettings();
    PaintSettings *getFillSettings();
    void updateDrawGradients();

    void setOutlineAffectedByScale(const bool &bT);

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);

    VectorPath *objectToVectorPathBox();
    VectorPath *strokeToVectorPathBox();

    const SkPath &getRelativePath() const;
    bool relPointInsidePath(const QPointF &relPos);

    void duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                    StrokeSettings *strokeSettings);

    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);

    void applyPaintSetting(const PaintSetting &setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);
    void updateStrokeDrawGradient();
    void updateFillDrawGradient();

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);

    void drawBoundingRectSk(SkCanvas *canvas,
                            const SkScalar &invScale);

    bool SWT_isPathBox() { return true; }

    void setupBoundingBoxRenderDataForRelFrameF(
                                const qreal &relFrame,
                                BoundingBoxRenderData* data);
    BoundingBoxRenderDataSPtr createRenderData() {
        return SPtrCreate(PathBoxRenderData)(this);
    }
    void updateCurrentPreviewDataFromRenderData(
            const BoundingBoxRenderDataSPtr& renderData);
    void duplicateStrokeSettingsFrom(
            StrokeSettings *strokeSettings);
    void duplicateFillSettingsFrom(
            PaintSettings *fillSettings);
    void duplicateStrokeSettingsNotAnimatedFrom(
            StrokeSettings *strokeSettings);
    void duplicateFillSettingsNotAnimatedFrom(
            PaintSettings *fillSettings);

    GradientPoints *getFillGradientPoints();
    GradientPoints *getStrokeGradientPoints();
    virtual SkPath getPathAtRelFrame(const int &relFrame) = 0;
    virtual SkPath getPathAtRelFrameF(const qreal &relFrame) = 0;
    SkPath getPathWithThisOnlyEffectsAtRelFrame(const int &relFrame);
    SkPath getPathWithEffectsUntilGroupSumAtRelFrame(const int &relFrame);
    SkPath getPathWithThisOnlyEffectsAtRelFrameF(const qreal &relFrame);
    SkPath getPathWithEffectsUntilGroupSumAtRelFrameF(const qreal &relFrame);

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void addPathEffect(const PathEffectQSPtr &effect);
    void addFillPathEffect(const PathEffectQSPtr &effect);
    void addOutlinePathEffect(const PathEffectQSPtr &effect);
    void removePathEffect(const PathEffectQSPtr &effect);
    void removeFillPathEffect(const PathEffectQSPtr &effect);
    void removeOutlinePathEffect(const PathEffectQSPtr &effect);
    PathEffectAnimators *getPathEffectsAnimators() {
        return mPathEffectsAnimators.data();
    }
    PathEffectAnimators *getFillPathEffectsAnimators() {
        return mFillPathEffectsAnimators.data();
    }
    PathEffectAnimators *getOutlinePathEffectsAnimators() {
        return mOutlinePathEffectsAnimators.data();
    }
    void copyPathBoxDataTo(PathBox *targetBox);

    virtual bool differenceInEditPathBetweenFrames(
            const int& frame1, const int& frame2) const = 0;
    bool differenceInPathBetweenFrames(
            const int& frame1, const int& frame2) const;
    bool differenceInOutlinePathBetweenFrames(
            const int& frame1, const int& frame2) const;
    bool differenceInFillPathBetweenFrames(
            const int& frame1, const int& frame2) const;
protected:
    PathBox(const BoundingBoxType &type);
    void getMotionBlurProperties(QList<Property*>& list);

    bool mOutlineAffectedByScale = true;

    int mCurrentPathsFrame = INT_MIN;

    SkPath mEditPathSk;
    SkPath mPathSk;
    SkPath mFillPathSk;
    SkPath mOutlinePathSk;

    PathEffectAnimatorsQSPtr mPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mFillPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mOutlinePathEffectsAnimators;
    GradientPointsQSPtr mFillGradientPoints;
    GradientPointsQSPtr mStrokeGradientPoints;

    PaintSettingsQSPtr mFillSettings;
    StrokeSettingsQSPtr mStrokeSettings;
};

#endif // PATHBOX_H
