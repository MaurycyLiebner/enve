#ifndef PATHBOX_H
#define PATHBOX_H
#include "Boxes/boundingbox.h"
#include "gradientpoints.h"
#include "Animators/paintsettings.h"

class PathBox : public BoundingBox
{
public:
    PathBox(BoxesGroup *parent, BoundingBoxType type);
    ~PathBox();

    void draw(QPainter *p);

    void schedulePathUpdate();

    void updatePathIfNeeded();

    void resetStrokeGradientPointsPos(bool finish) {
        mStrokeGradientPoints.prp_setRecording(false);
        mStrokeGradientPoints.setPositions(mRelBoundingRect.topLeft(),
                                           mRelBoundingRect.bottomRight(),
                                           finish);
    }

    void resetFillGradientPointsPos(bool finish) {
        mFillGradientPoints.prp_setRecording(false);
        mFillGradientPoints.setPositions(mRelBoundingRect.topLeft(),
                                         mRelBoundingRect.bottomRight(),
                                         finish);
    }

    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        mStrokeSettings.setCapStyle(capStyle);
        scheduleOutlinePathUpdate();
    }

    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        mStrokeSettings.setJoinStyle(joinStyle);
        scheduleOutlinePathUpdate();
    }

    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        mStrokeSettings.setCurrentStrokeWidth(strokeWidth);
        if(finish) {
            mStrokeSettings.getStrokeWidthAnimator()->prp_finishTransform();
        }
        //scheduleOutlinePathUpdate();
    }

    void setOutlineCompositionMode(QPainter::CompositionMode compositionMode) {
        mStrokeSettings.setOutlineCompositionMode(compositionMode);
        scheduleUpdate();
    }

    void startSelectedStrokeWidthTransform() {
        mStrokeSettings.getStrokeWidthAnimator()->prp_startTransform();
    }

    void startSelectedStrokeColorTransform() {
        mStrokeSettings.getColorAnimator()->prp_startTransform();
    }

    void startSelectedFillColorTransform() {
        mFillPaintSettings.getColorAnimator()->prp_startTransform();
    }

    StrokeSettings *getStrokeSettings();
    PaintSettings *getFillSettings();
    void updateDrawGradients();

    void updateOutlinePath();
    void scheduleOutlinePathUpdate();
    void updateOutlinePathIfNeeded();

    void setOutlineAffectedByScale(bool bT);
    int prp_saveToSql(QSqlQuery *query, const int &parentId);
    void loadFromSql(int boundingBoxId);
    void updateRelBoundingRect();

    void setUpdateVars();
    void copyStrokeAndFillSettingsTo(PathBox *target);

    VectorPath *objectToPath();
    const QPainterPath &getRelativePath() const { return mPath; }
    bool relPointInsidePath(QPointF relPos);
    void preUpdatePixmapsUpdates();

    void duplicateGradientPointsFrom(GradientPoints *fillGradientPoints,
                                     GradientPoints *strokeGradientPoints);
    void duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                    StrokeSettings *strokeSettings);

    void makeDuplicate(BoundingBox *targetBox);

    virtual void drawHovered(QPainter *p) {
        p->save();
        //p->setCompositionMode(QPainter::CompositionMode_Difference);
        p->setTransform(QTransform(mCombinedTransformMatrix), true);
        QPen pen = QPen(Qt::red, 1.);
        pen.setCosmetic(true);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        p->drawPath(mPath);
        p->restore();
    }

    void applyPaintSetting(
            const PaintSetting &setting) {
        setting.apply(this);
        scheduleUpdate();
    }

    void setFillColorMode(const ColorMode &colorMode) {
        mFillPaintSettings.getColorAnimator()->setColorMode(colorMode);
    }
    void setStrokeColorMode(const ColorMode &colorMode) {
        mFillPaintSettings.getColorAnimator()->setColorMode(colorMode);
    }
    void updateStrokeDrawGradient();
    void updateFillDrawGradient();

    virtual void updatePath() {}
protected:
    GradientPoints mFillGradientPoints;
    GradientPoints mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokeSettings;


    bool mPathUpdateNeeded = false;
    bool mOutlinePathUpdateNeeded = false;

    QPainterPath mUpdatePath;
    QPainterPath mUpdateOutlinePath;
    QPainterPath mPath;
    QPainterPath mOutlinePath;
    QPainterPathStroker mPathStroker;
    QPainterPath mWholePath;
    void updateWholePath();

    bool mOutlineAffectedByScale = true;
};

#endif // PATHBOX_H
