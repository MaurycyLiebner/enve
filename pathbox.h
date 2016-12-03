#ifndef PATHBOX_H
#define PATHBOX_H
#include "boundingbox.h"
#include "gradientpoints.h"

class PathBox : public BoundingBox
{
public:
    PathBox(BoxesGroup *parent, BoundingBoxType type);
    ~PathBox();

    QRectF getPixBoundingRect();
    void draw(QPainter *p);

    void schedulePathUpdate();

    void updatePathIfNeeded();

    void resetStrokeGradientPointsPos(bool finish) {
        mStrokeGradientPoints.setRecording(false);
        mStrokeGradientPoints.setPositions(getPixBoundingRect().topLeft(),
                     getPixBoundingRect().bottomRight(), finish);
    }

    void resetFillGradientPointsPos(bool finish) {
        mFillGradientPoints.setRecording(false);
        mFillGradientPoints.setPositions(getPixBoundingRect().topLeft(),
                     getPixBoundingRect().bottomRight(), finish);
    }

    void changeFillGradient(bool wasGradient, bool isGradient, bool finish) {
        if(wasGradient) {
            mFillPaintSettings.getGradient()->removePath(this);
        } else if(isGradient) {
            resetFillGradientPointsPos(finish);
        }
    }

    void changeStrokeGradient(bool wasGradient, bool isGradient, bool finish) {
        if(wasGradient) {
            mStrokeSettings.getGradient()->removePath(this);
        } else if(isGradient) {
            resetStrokeGradientPointsPos(finish);
        }
    }

    virtual void setFillGradient(Gradient* gradient, bool finish) {
        if(mFillPaintSettings.getPaintType() != GRADIENTPAINT) {
            setFillPaintType(GRADIENTPAINT,
                              mFillPaintSettings.getCurrentColor(),
                              gradient);
        } else {
            changeFillGradient(mFillPaintSettings.getPaintType() == GRADIENTPAINT,
                                 true, finish);

            mFillPaintSettings.setGradient(gradient);
            updateDrawGradients();
        }
        scheduleAwaitUpdate();
    }

    virtual void setStrokeGradient(Gradient* gradient, bool finish) {
        if(mStrokeSettings.getPaintType() != GRADIENTPAINT) {
            setStrokePaintType(GRADIENTPAINT,
                               mStrokeSettings.getCurrentColor(),
                               gradient);
        } else {
            changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                                 true, finish);

            mStrokeSettings.setGradient(gradient);
            updateDrawGradients();
        }
        scheduleAwaitUpdate();
    }

    virtual void setFillFlatColor(Color color, bool finish) {
        if(mFillPaintSettings.getPaintType() != FLATPAINT) {
            setFillPaintType(FLATPAINT, color, NULL);
        } else {
            mFillPaintSettings.setCurrentColor(color);
            if(finish) {
                mFillPaintSettings.getColorAnimator()->finishTransform();
            }
        }

        scheduleAwaitUpdate();
    }

    virtual void setStrokeFlatColor(Color color, bool finish) {
        if(mStrokeSettings.getPaintType() != FLATPAINT) {
            setStrokePaintType(FLATPAINT, color, NULL);
        } else {
            mStrokeSettings.setCurrentColor(color);
            if(finish) {
                mStrokeSettings.getColorAnimator()->finishTransform();
            }
        }

        scheduleAwaitUpdate();
    }

    virtual void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient) {
        changeFillGradient(mFillPaintSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mFillPaintSettings.setGradient(gradient);
        } else if(paintType == FLATPAINT) {
            mFillPaintSettings.setCurrentColor(color);
        }
        mFillPaintSettings.setPaintType(paintType);

        updateDrawGradients();

        scheduleAwaitUpdate();
    }

    virtual void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient) {
        changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mStrokeSettings.setGradient(gradient);
        } else if(paintType == FLATPAINT) {
            mStrokeSettings.setCurrentColor(color);
        }
        mStrokeSettings.setPaintType(paintType);

        updateDrawGradients();

        scheduleAwaitUpdate();
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
            mStrokeSettings.getStrokeWidthAnimator()->finishTransform();
        }
        //scheduleOutlinePathUpdate();
    }

    void setOutlineCompositionMode(QPainter::CompositionMode compositionMode) {
        mStrokeSettings.setOutlineCompositionMode(compositionMode);
        scheduleAwaitUpdate();
    }

    void startStrokeWidthTransform() {
        mStrokeSettings.getStrokeWidthAnimator()->startTransform();
    }

    void startStrokeColorTransform() {
        mStrokeSettings.getColorAnimator()->startTransform();
    }

    void startFillColorTransform() {
        mFillPaintSettings.getColorAnimator()->startTransform();
    }

    const StrokeSettings *getStrokeSettings();
    const PaintSettings *getFillSettings();
    void updateDrawGradients();

    void updateOutlinePath();
    void scheduleOutlinePathUpdate();
    void updateOutlinePathIfNeeded();

    bool absPointInsidePath(QPointF absPoint);
    void updateAfterFrameChanged(int currentFrame);

    void setOutlineAffectedByScale(bool bT);
    int saveToSql(int parentId);
    void loadFromSql(int boundingBoxId);
    virtual void updateBoundingRect();
    void updatePixmaps();
    void updatePixBoundingRectClippedToView();

    void afterSuccessfulUpdate();

    void updateUpdateTransform();
    void render(QPainter *p);
    void renderFinal(QPainter *p);
protected:
    GradientPoints mFillGradientPoints;
    GradientPoints mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokeSettings;


    bool mPathUpdateNeeded = false;
    bool mOutlinePathUpdateNeeded = false;

    bool mUpdatePathUpdateNeeded = false;
    bool mUpdateOutlinePathUpdateNeeded = false;

    QPainterPath mPath;
    QPainterPath mOutlinePath;
    QPainterPathStroker mPathStroker;
    QPainterPath mWholePath;
    void updateWholePath();
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();

    virtual void updatePath() {}

    bool mOutlineAffectedByScale = true;
};

#endif // PATHBOX_H
