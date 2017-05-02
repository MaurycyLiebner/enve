#ifndef PATHBOX_H
#define PATHBOX_H
#include "Boxes/boundingbox.h"
#include "gradientpoints.h"
#include "Animators/paintsettings.h"

class PathBox : public BoundingBox
{
public:
    PathBox(BoxesGroup *parent,
            const BoundingBoxType &type);
    ~PathBox();

    void draw(QPainter *p);

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

    virtual void drawHovered(QPainter *p);

    void applyPaintSetting(
            const PaintSetting &setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);
    void updateStrokeDrawGradient();
    void updateFillDrawGradient();

    virtual void updatePath();
protected:
    QSharedPointer<GradientPoints> mFillGradientPoints =
            (new GradientPoints)->ref<GradientPoints>();
    QSharedPointer<GradientPoints> mStrokeGradientPoints =
            (new GradientPoints)->ref<GradientPoints>();


    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;

    QSharedPointer<PaintSettings> mFillSettings =
            (new PaintSettings)->ref<PaintSettings>();
    QSharedPointer<StrokeSettings> mStrokeSettings =
            (new StrokeSettings)->ref<StrokeSettings>();


    bool mPathUpdateNeeded = false;
    bool mOutlinePathUpdateNeeded = false;

    QPainterPath mUpdatePath;
    QPainterPath mUpdateOutlinePath;
    bool mFillSettingsGradientUpdateNeeded = false;
    bool mStrokeSettingsGradientUpdateNeeded = false;
    UpdatePaintSettings mUpdateFillSettings;
    UpdateStrokeSettings mUpdateStrokeSettings;

    QPainterPath mPath;
    QPainterPath mOutlinePath;
    QPainterPath mWholePath;
    void updateWholePath();

    bool mOutlineAffectedByScale = true;
};

#endif // PATHBOX_H
