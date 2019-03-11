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
class GradientPoints;
class SkStroke;
class PathEffectAnimators;
class PathEffect;

struct PathBoxRenderData : public BoundingBoxRenderData {
    PathBoxRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
    }

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
        fRelBoundingRect = SkRectToQRectF(totalPath.computeTightBounds());
    }
    QPointF getCenterPosition() {
        return SkRectToQRectF(fEditPath.getBounds()).center();
    }
protected:
    void drawSk(SkCanvas *canvas) {
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
                fPath.transform(QMatrixToSkMatrix(trans), &pathT);
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
                            widthCurve, 5, 5);
                QColor col = fStrokeSettings.fPaintColor;
                col.setRgbF(col.blueF(), col.greenF(),
                            col.redF(), col.alphaF());
                fStrokeSettings.fStrokeBrush->setColor(
                            qrealToSkScalar(col.hueF()),
                            qrealToSkScalar(col.saturationF()),
                            qrealToSkScalar(col.valueF()));
                const auto brush = fStrokeSettings.fStrokeBrush->getBrush();
                for(auto& set : brushSet) {
                    surf.execute(brush, set);
                }

                fBitmapTMP.reset();
                const int iMargin = qCeil(fEffectsMargin);
                fBitmapTMP = surf.toBitmap(iMargin);
                fGlobalBoundingRect.translate(-surf.zeroTilePos() -
                                              QPoint(iMargin, iMargin));
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
public:
    ~PathBox();

    void resetStrokeGradientPointsPos();

    void resetFillGradientPointsPos();

    void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setStrokeWidth(const qreal &strokeWidth);

    void setStrokeBrush(_SimpleBrushWrapper * const brush) {
        mStrokeSettings->setStrokeBrush(brush);
        clearAllCache();
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

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    void startSelectedStrokeWidthTransform();

    void startSelectedStrokeColorTransform();

    void startSelectedFillColorTransform();

    StrokeSettings *getStrokeSettings() const;
    PaintSettings *getFillSettings() const;
    void updateDrawGradients();

    void setOutlineAffectedByScale(const bool &bT);

    QRectF getRelBoundingRectAtRelFrame(const qreal &relFrame);

    VectorPath *objectToVectorPathBox();
    VectorPath *strokeToVectorPathBox();

    const SkPath &getRelativePath() const;
    bool relPointInsidePath(const QPointF &relPos) const;

    void duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                    StrokeSettings *strokeSettings);

    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);

    void applyPaintSetting( PaintSetting *setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);
    void updateStrokeDrawGradient();
    void updateFillDrawGradient();

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);

    void drawBoundingRectSk(SkCanvas *canvas,
                            const SkScalar &invScale);

    bool SWT_isPathBox() const { return true; }

    void setupBoundingBoxRenderDataForRelFrameF(
                                const qreal &relFrame,
                                BoundingBoxRenderData* data);
    stdsptr<BoundingBoxRenderData> createRenderData() {
        return SPtrCreate(PathBoxRenderData)(this);
    }
    void updateCurrentPreviewDataFromRenderData(BoundingBoxRenderData *renderData);
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
    virtual SkPath getPathAtRelFrameF(const qreal &relFrame) = 0;
    SkPath getPathWithThisOnlyEffectsAtRelFrameF(const qreal &relFrame);
    SkPath getPathWithEffectsUntilGroupSumAtRelFrameF(const qreal &relFrame);

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void addPathEffect(const qsptr<PathEffect> &effect);
    void addFillPathEffect(const qsptr<PathEffect> &effect);
    void addOutlinePathEffect(const qsptr<PathEffect> &effect);
    void removePathEffect(const qsptr<PathEffect> &effect);
    void removeFillPathEffect(const qsptr<PathEffect> &effect);
    void removeOutlinePathEffect(const qsptr<PathEffect> &effect);
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
    void setPathsOutdated() {
        mCurrentPathsOutdated = true;
    }

    void setOutlinePathOutdated() {
        mCurrentOutlinePathOutdated = true;
    }

//    void updateDialog_TEST() {
//        if(!mDialog_TEST) {
//            mDialog_TEST = new QDialog();
//            mDialog_TEST->open();
//            mDialog_TEST->setLayout(new QVBoxLayout());
//            mLabel_TEST = new QLabel();
//            mDialog_TEST->layout()->addWidget(mLabel_TEST);
//        }
//        QPixmap pix(200, 200);
//        pix.fill(Qt::white);
//        QPainter p(&pix);
//        p.setPen(Qt::NoPen);
//        p.setBrush(Qt::red);
//        p.drawPath(SkPathToQPainterPath(mFillPathSk));
//        p.setBrush(Qt::black);
//        auto oPath = SkPathToQPainterPath(mOutlinePathSk);
//        oPath.setFillRule(Qt::WindingFill);
//        p.drawPath(oPath);
//        p.end();

//        mLabel_TEST->setPixmap(pix);
    //    }
protected:
    PathBox(const BoundingBoxType &type);
    void getMotionBlurProperties(QList<Property*> &list) const;

//    QDialog* mDialog_TEST = nullptr;
//    QLabel* mLabel_TEST = nullptr;

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

    qsptr<PaintSettings> mFillSettings;
    qsptr<StrokeSettings> mStrokeSettings;
};

#endif // PATHBOX_H
