#ifndef PATHBOX_H
#define PATHBOX_H
#include "Boxes/boundingbox.h"
#include "Animators/paintsettings.h"
class GradientPoints;
class SkStroke;
typedef QSharedPointer<GradientPoints> GradientPointsQSPtr;

class PathBox : public BoundingBox {
public:
    PathBox(BoxesGroup *parent,
            const BoundingBoxType &type);
    ~PathBox();

    void drawSk(SkCanvas *canvas);

    void schedulePathUpdate();

    void updatePathIfNeeded();

    void resetStrokeGradientPointsPos(bool finish);

    void resetFillGradientPointsPos(bool finish);

    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle);

    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle);

    virtual void setStrokeWidth(qreal strokeWidth, bool finish);

    void setOutlineCompositionMode(QPainter::CompositionMode compositionMode);

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
    int prp_saveToSql(QSqlQuery *query, const int &parentId);
    void prp_loadFromSql(const int &boundingBoxId);
    void updateRelBoundingRect();

    void setUpdateVars();

    VectorPath *objectToPath();
    VectorPath *strokeToPath();

    const QPainterPath &getRelativePath() const;
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
protected:
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

    QPainterPath mUpdatePath;
    SkPath mUpdatePathSk;
    QPainterPath mUpdateOutlinePath;
    SkPath mUpdateOutlinePathSk;
    bool mFillSettingsGradientUpdateNeeded = false;
    bool mStrokeSettingsGradientUpdateNeeded = false;
    UpdatePaintSettings mUpdateFillSettings;
    UpdateStrokeSettings mUpdateStrokeSettings;

    SkPath mPathSk;
    SkPath mOutlinePathSk;
    SkPath mWholePathSk;
    QPainterPath mPath;
    QPainterPath mOutlinePath;
    QPainterPath mWholePath;
    void updateWholePathSk();

    bool mOutlineAffectedByScale = true;
};

#endif // PATHBOX_H
