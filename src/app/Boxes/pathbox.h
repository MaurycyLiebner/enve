#ifndef PATHBOX_H
#define PATHBOX_H
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include "Boxes/boundingbox.h"
#include "Animators/paintsettingsanimator.h"
#include "canvas.h"
#include "Paint/autotiledsurface.h"
#include "GUI/BrushWidgets/brushwidget.h"
#include <mypaint-brush.h>
class SmartVectorPath;
class GradientPoints;
class SkStroke;
class PathEffectAnimators;
class PathEffect;

struct PathBoxRenderData : public BoxRenderData {
    PathBoxRenderData(BoundingBox * const parentBoxT) :
        BoxRenderData(parentBoxT) {}

    SkPath fEditPath;
    SkPath fPath;
    SkPath fFillPath;
    SkPath fOutlinePath;
    UpdatePaintSettings fPaintSettings;
    UpdateStrokeSettings fStrokeSettings;

    void updateRelBoundingRect() {
        SkPath totalPath;
        totalPath.addPath(fFillPath);
        totalPath.addPath(fOutlinePath);
        fRelBoundingRect = toQRectF(totalPath.computeTightBounds());
    }
    QPointF getCenterPosition() {
        return toQRectF(fEditPath.getBounds()).center();
    }
protected:
    void drawSk(SkCanvas * const canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        if(!fFillPath.isEmpty()) {
            fPaintSettings.applyPainterSettingsSk(&paint);
            canvas->drawPath(fFillPath, paint);
        }
        if(!fOutlinePath.isEmpty()) {
            paint.setShader(nullptr);
            fStrokeSettings.applyPainterSettingsSk(&paint);
            canvas->drawPath(fOutlinePath, paint);
        }
    }
};

class PathBox : public BoundingBox {
    friend class SelfRef;
protected:
    PathBox(const BoundingBoxType &type);
    void getMotionBlurProperties(QList<Property*> &list) const;
public:
    ~PathBox();

    virtual bool differenceInEditPathBetweenFrames(
            const int frame1, const int frame2) const = 0;
    virtual SkPath getPathAtRelFrameF(const qreal relFrame) = 0;
    void setParentGroup(ContainerBox * const parent);

    void addPathEffect(const qsptr<PathEffect> &effect);
    void addFillPathEffect(const qsptr<PathEffect> &effect);
    void addOutlineBasePathEffect(const qsptr<PathEffect> &effect);
    void addOutlinePathEffect(const qsptr<PathEffect> &effect);
    void removePathEffect(const qsptr<PathEffect> &effect);
    void removeFillPathEffect(const qsptr<PathEffect> &effect);
    void removeOutlinePathEffect(const qsptr<PathEffect> &effect);

    void setStrokeCapStyle(const SkPaint::Cap capStyle);
    void setStrokeJoinStyle(const SkPaint::Join joinStyle);
    void strokeWidthAction(const QrealAction &action);

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    OutlineSettingsAnimator *getStrokeSettings() const;
    FillSettingsAnimator *getFillSettings() const;

    QRectF getRelBoundingRect(const qreal relFrame);

    SmartVectorPath *objectToVectorPathBox();
    SmartVectorPath *strokeToVectorPathBox();

    bool relPointInsidePath(const QPointF &relPos) const;

    void drawHoveredSk(SkCanvas *canvas, const float invScale);

    void applyPaintSetting(const PaintSettingsApplier &setting);

    void setFillColorMode(const ColorMode colorMode);
    void setStrokeColorMode(const ColorMode colorMode);

    bool SWT_isPathBox() const { return true; }

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);
    stdsptr<BoxRenderData> createRenderData() {
        return SPtrCreate(PathBoxRenderData)(this);
    }
    void updateCurrentPreviewDataFromRenderData(
            BoxRenderData *renderData);

    void duplicateStrokeSettingsFrom(
            OutlineSettingsAnimator * const strokeSettings);
    void duplicateFillSettingsFrom(
            FillSettingsAnimator * const fillSettings);
    void duplicateStrokeSettingsNotAnimatedFrom(
            OutlineSettingsAnimator * const strokeSettings);
    void duplicateFillSettingsNotAnimatedFrom(
            FillSettingsAnimator * const fillSettings);
    void duplicatePaintSettingsFrom(FillSettingsAnimator * const fillSettings,
                                    OutlineSettingsAnimator * const strokeSettings);

    void updateStrokeDrawGradient();
    void updateFillDrawGradient();
    const SkPath &getRelativePath() const;
    void updateDrawGradients();
    void setOutlineAffectedByScale(const bool bT);

    GradientPoints *getFillGradientPoints();
    GradientPoints *getStrokeGradientPoints();

    SkPath getPathWithThisOnlyEffectsAtRelFrameF(const qreal relFrame);

    void setPathEffectsEnabled(const bool enable);
    bool getPathEffectsEnabled() const;

    void setFillEffectsEnabled(const bool enable);
    bool getFillEffectsEnabled() const;

    void setOutlineBaseEffectsEnabled(const bool enable);
    bool getOutlineBaseEffectsEnabled() const;

    void setOutlineEffectsEnabled(const bool enable);
    bool getOutlineEffectsEnabled() const;

    PathEffectAnimators *getPathEffectsAnimators() {
        return mPathEffectsAnimators.data();
    }

    PathEffectAnimators *getFillPathEffectsAnimators() {
        return mFillPathEffectsAnimators.data();
    }

    PathEffectAnimators *getOutlineBasrPathEffectsAnimators() {
        return mOutlineBasePathEffectsAnimators.data();
    }

    PathEffectAnimators *getOutlinePathEffectsAnimators() {
        return mOutlinePathEffectsAnimators.data();
    }
    void copyPathBoxDataTo(PathBox * const targetBox);

    bool differenceInPathBetweenFrames(
            const int frame1, const int frame2) const;
    bool differenceInOutlinePathBetweenFrames(
            const int frame1, const int frame2) const;
    bool differenceInFillPathBetweenFrames(
            const int frame1, const int frame2) const;
    void setPathsOutdated() {
        mCurrentPathsOutdated = true;
    }

    void resetStrokeGradientPointsPos();
    void resetFillGradientPointsPos();

    void setOutlinePathOutdated() {
        mCurrentOutlinePathOutdated = true;
    }
    void setupCanvasMenu(PropertyMenu * const menu);
protected:
    bool mOutlineAffectedByScale = true;
    bool mCurrentPathsOutdated = true;
    bool mCurrentOutlinePathOutdated = true;

    int mCurrentPathsFrame = 0;

    SkPath mEditPathSk;
    SkPath mPathSk;
    SkPath mFillPathSk;
    SkPath mOutlinePathSk;

    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlineBasePathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
    qsptr<GradientPoints> mFillGradientPoints;
    qsptr<GradientPoints> mStrokeGradientPoints;

    qsptr<FillSettingsAnimator> mFillSettings;
    qsptr<OutlineSettingsAnimator> mStrokeSettings;
};

#endif // PATHBOX_H
