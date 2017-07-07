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
    SkPath path;
    SkPath outlinePath;
    UpdatePaintSettings paintSettings;
    UpdateStrokeSettings strokeSettings;
private:
    void drawSk(SkCanvas *canvas) {
        canvas->save();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        if(!path.isEmpty()) {
            paintSettings.applyPainterSettingsSk(&paint);

            canvas->drawPath(path, paint);
        }
        if(!outlinePath.isEmpty()) {
            paint.setShader(NULL);
            strokeSettings.applyPainterSettingsSk(&paint);

            canvas->drawPath(outlinePath, paint);
        }

        canvas->restore();
    }
};

class PathBox : public BoundingBox {
public:
    PathBox(BoxesGroup *parent,
            const BoundingBoxType &type);
    ~PathBox();

    void schedulePathUpdate();

    void updatePathIfNeeded();

    void resetStrokeGradientPointsPos(bool finish);

    void resetFillGradientPointsPos(bool finish);

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

    void updateOutlinePath();
    void updateOutlinePathSk();
    void scheduleOutlinePathUpdate();
    void updateOutlinePathIfNeeded();

    void setOutlineAffectedByScale(bool bT);
    int saveToSql(QSqlQuery *query, const int &parentId);
    void prp_loadFromSql(const int &boundingBoxId);
    void updateRelBoundingRect();

    void setUpdateVars();

    VectorPath *objectToPath();
    VectorPath *strokeToPath();

    const SkPath &getRelativePath() const;
    bool relPointInsidePath(const QPointF &relPos);
    void preUpdatePixmapsUpdates();

    void duplicateGradientPointsFrom(GradientPoints *fillGradientPoints,
                                     GradientPoints *strokeGradientPoints);
    void duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                    StrokeSettings *strokeSettings);

    void makeDuplicate(Property *targetBox);

    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);

    void applyPaintSetting(
            const PaintSetting &setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);
    void updateStrokeDrawGradient();
    void updateFillDrawGradient();

    virtual void updatePath() {}
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);

    bool SWT_isPathBox() { return true; }

    void addPathEffect(PathEffect *effect);
    void addOutlinePathEffect(PathEffect *effect);
    void updateEffectsMargin();

    void setupBoundingBoxRenderDataForRelFrame(
                                const int &relFrame,
                                BoundingBoxRenderData *data);

    void createCurrentRenderData() {
        mCurrentRenderData = new PathBoxRenderData();
    }
protected:
    PathEffectAnimatorsQSPtr mPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mOutlinePathEffectsAnimators;
    GradientPointsQSPtr mFillGradientPoints;
    GradientPointsQSPtr mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;

    QSharedPointer<PaintSettings> mFillSettings =
            (new PaintSettings)->ref<PaintSettings>();
    QSharedPointer<StrokeSettings> mStrokeSettings =
            (new StrokeSettings)->ref<StrokeSettings>();


    bool mPathUpdateNeeded = false;
    bool mOutlinePathUpdateNeeded = false;

    SkPath mUpdatePathSk;
    SkPath mUpdateOutlinePathSk;
    bool mFillSettingsGradientUpdateNeeded = false;
    bool mStrokeSettingsGradientUpdateNeeded = false;
    UpdatePaintSettings mUpdateFillSettings;
    UpdateStrokeSettings mUpdateStrokeSettings;

    SkPath mPathSk;
    SkPath mOutlinePathSk;
    SkPath mWholePathSk;
    QPainterPath mPath;
    void updateWholePathSk();

    bool mOutlineAffectedByScale = true;
};

#endif // PATHBOX_H
