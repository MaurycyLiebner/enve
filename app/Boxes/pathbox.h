#ifndef PATHBOX_H
#define PATHBOX_H
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include "Boxes/boundingbox.h"
#include "Animators/paintsettings.h"
#include "canvas.h"
#include "Paint/autotiledsurface.h"
#include "GUI/BrushWidgets/brushwidget.h"
#include <mypaint-brush.h>
class SmartVectorPath;
class GradientPoints;
class SkStroke;
class PathEffectAnimators;
class PathEffect;

struct PathBoxRenderData : public BoundingBoxRenderData {
    PathBoxRenderData(BoundingBox * const parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {}

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
        canvas->save();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        if(!fFillPath.isEmpty()) {
            fPaintSettings.applyPainterSettingsSk(&paint);

            canvas->drawPath(fFillPath, paint);
        }
        if(!fOutlinePath.isEmpty()) {
            if(fStrokeSettings.fPaintType == PaintType::BRUSHPAINT
                    /*strokeSettings.fStrokeBrush*/) {
                if(!fStrokeSettings.fStrokeBrush) {
                    canvas->restore();
                    return;
                }
                canvas->resetMatrix();
                AutoTiledSurface surf;
                surf.loadBitmap(fBitmapTMP);

                SkPath pathT;
                QMatrix trans;
                trans.translate(-fGlobalBoundingRect.left(),
                                -fGlobalBoundingRect.top());
                trans = fScaledTransform*trans;
                fPath.transform(toSkMatrix(trans), &pathT);
//                auto brushSet = BrushStrokeSet::fillStrokesForSkPath(pathT, 5);
//                brush->setColor(0, 0, 1);
//                for(auto& set : brushSet) {
//                    set.execute(brush->getItem(), &surf, 5);
//                }
                auto widthCurve = fStrokeSettings.fWidthCurve*fResolution;
                auto brushSet = BrushStrokeSet::outlineStrokesForSkPath(
                            pathT,
                            fStrokeSettings.fTimeCurve,
                            fStrokeSettings.fPressureCurve,
                            widthCurve, fStrokeSettings.fSpacingCurve, 5, 5);
                QColor col = fStrokeSettings.fPaintColor;
                col.setRgbF(col.blueF(), col.greenF(),
                            col.redF(), col.alphaF());
                fStrokeSettings.fStrokeBrush->setColor(
                            toSkScalar(col.hueF()),
                            toSkScalar(col.saturationF()),
                            toSkScalar(col.valueF()));
                const auto brush = fStrokeSettings.fStrokeBrush->getBrush();
                for(auto& set : brushSet) surf.execute(brush, set);

                fBitmapTMP.reset();
                const int iMargin = qCeil(fEffectsMargin);
                fBitmapTMP = surf.toBitmap(iMargin);
                fGlobalBoundingRect.translate(-surf.zeroTilePos() -
                                              QPoint(iMargin, iMargin));
                fGlobalBoundingRect.setSize(QSizeF(fBitmapTMP.width(),
                                                   fBitmapTMP.height()));
                fixupGlobalBoundingRect();
            } else {
                paint.setShader(nullptr);
                fStrokeSettings.applyPainterSettingsSk(&paint);

                canvas->drawPath(fOutlinePath, paint);
            }
        }

        canvas->restore();
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
            const int& frame1, const int& frame2) const = 0;
    virtual SkPath getPathAtRelFrameF(const qreal &relFrame) = 0;
    void setParentGroup(BoxesGroup * const parent);

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void drawCanvasControls(SkCanvas * const canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale);
    void addPathEffect(const qsptr<PathEffect> &effect);
    void addFillPathEffect(const qsptr<PathEffect> &effect);
    void addOutlinePathEffect(const qsptr<PathEffect> &effect);
    void removePathEffect(const qsptr<PathEffect> &effect);
    void removeFillPathEffect(const qsptr<PathEffect> &effect);
    void removeOutlinePathEffect(const qsptr<PathEffect> &effect);

    void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setStrokeWidth(const qreal &strokeWidth);

    void setStrokeBrush(SimpleBrushWrapper * const brush) {
        mStrokeSettings->setStrokeBrush(brush);
        prp_updateInfluenceRangeAfterChanged();
        scheduleUpdate(Animator::USER_CHANGE);
    }
    void setStrokeBrushWidthCurve(
            const qCubicSegment1D& curve) {
        mStrokeSettings->setStrokeBrushWidthCurve(curve);
    }
    void setStrokeBrushTimeCurve(
            const qCubicSegment1D& curve) {
        mStrokeSettings->setStrokeBrushTimeCurve(curve);
    }
    void setStrokeBrushPressureCurve(
            const qCubicSegment1D& curve) {
        mStrokeSettings->setStrokeBrushPressureCurve(curve);
    }
    void setStrokeBrushSpacingCurve(
            const qCubicSegment1D& curve) {
        mStrokeSettings->setStrokeBrushSpacingCurve(curve);
    }

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    void startSelectedStrokeWidthTransform();

    void startSelectedStrokeColorTransform();

    void startSelectedFillColorTransform();

    OutlineSettingsAnimator *getStrokeSettings() const;
    FillSettingsAnimator *getFillSettings() const;

    QRectF getRelBoundingRectAtRelFrame(const qreal &relFrame);

    SmartVectorPath *objectToVectorPathBox();
    SmartVectorPath *strokeToVectorPathBox();

    bool relPointInsidePath(const QPointF &relPos) const;

    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);

    void applyPaintSetting(const PaintSettingsApplier &setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);

    void drawBoundingRect(SkCanvas * const canvas,
                            const SkScalar &invScale);

    bool SWT_isPathBox() const { return true; }

    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
    stdsptr<BoundingBoxRenderData> createRenderData() {
        return SPtrCreate(PathBoxRenderData)(this);
    }
    void updateCurrentPreviewDataFromRenderData(
            BoundingBoxRenderData *renderData);

    void addActionsToMenu(BoxTypeMenu * const menu);

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
    void setOutlineAffectedByScale(const bool &bT);

    GradientPoints *getFillGradientPoints();
    GradientPoints *getStrokeGradientPoints();

    SkPath getPathWithThisOnlyEffectsAtRelFrameF(const qreal &relFrame);

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

    bool differenceInPathBetweenFrames(
            const int& frame1, const int& frame2) const;
    bool differenceInOutlinePathBetweenFrames(
            const int& frame1, const int& frame2) const;
    bool differenceInFillPathBetweenFrames(
            const int& frame1, const int& frame2) const;
    void setPathsOutdated() {
        mCurrentPathsOutdated = true;
    }

    void resetStrokeGradientPointsPos();
    void resetFillGradientPointsPos();

    void setOutlinePathOutdated() {
        mCurrentOutlinePathOutdated = true;
    }
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
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
    qsptr<GradientPoints> mFillGradientPoints;
    qsptr<GradientPoints> mStrokeGradientPoints;

    qsptr<FillSettingsAnimator> mFillSettings;
    qsptr<OutlineSettingsAnimator> mStrokeSettings;
private:
    template<typename T, typename U>
    void addPathEffectActionToMenu(const QString& text,
                                   BoxTypeMenu * const menu,
                                   const U& adder,
                                   const bool& outline);
    template <typename U>
    void addPathEffectsActionToMenu(BoxTypeMenu * const menu,
                                    const U& adder,
                                    const bool& outline);
};

#endif // PATHBOX_H
