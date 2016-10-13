#ifndef PATHBOX_H
#define PATHBOX_H
#include "boundingbox.h"
#include "gradientpoints.h"

class PathBox : public BoundingBox
{
public:
    PathBox(BoxesGroup *parent, BoundingBoxType type);
    ~PathBox();

    QRectF getBoundingRect();
    void draw(QPainter *p);

    void schedulePathUpdate();
    void scheduleMappedPathUpdate();

    void updatePathIfNeeded();
    void updateMappedPathIfNeeded();

    void resetStrokeGradientPointsPos(bool finish) {
        mStrokeGradientPoints.setRecording(false);
        mStrokeGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), finish);
    }

    void resetFillGradientPointsPos(bool finish) {
        mFillGradientPoints.setRecording(false);
        mFillGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), finish);
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
    }

    virtual void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient) {
        changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mStrokeSettings.setGradient(gradient);
            updateDrawGradients();
        } else if(paintType == FLATPAINT) {
            mStrokeSettings.setCurrentColor(color);
        }
        mStrokeSettings.setPaintType(paintType);


    }

    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        mStrokeSettings.setCapStyle(capStyle);
        updateOutlinePath();


    }

    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        mStrokeSettings.setJoinStyle(joinStyle);
        updateOutlinePath();

    }

    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        mStrokeSettings.setCurrentStrokeWidth(strokeWidth);
        if(finish) {
            mStrokeSettings.getStrokeWidthAnimator()->finishTransform();
        }
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
    void setRenderCombinedTransform();

    bool pointInsidePath(QPointF point);
    void updateAfterFrameChanged(int currentFrame);

    void setOutlineAffectedByScale(bool bT);
protected:
    GradientPoints mFillGradientPoints;
    GradientPoints mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokeSettings;


    bool mPathUpdateNeeded = false;
    bool mMappedPathUpdateNeeded = false;
    bool mOutlinePathUpdateNeeded = false;


    QPainterPath mPath;
    QPainterPath mMappedEditPath;
    QPainterPath mMappedPath;
    QPainterPath mMappedOutlinePath;
    QPainterPathStroker mPathStroker;
    QPainterPath mMappedWhole;
    void updateWholePath();
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();

    virtual void updatePath() {}
    virtual void updateMappedPath();

    bool mOutlineAffectedByScale = true;
};

#endif // PATHBOX_H
