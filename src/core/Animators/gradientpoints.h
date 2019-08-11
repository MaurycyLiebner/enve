#ifndef GRADIENTPOINTS_H
#define GRADIENTPOINTS_H
#include "Animators/staticcomplexanimator.h"
class GradientPoint;
class MovablePoint;
class PathBox;
class QPointFAnimator;
#include "skia/skiaincludes.h"

class GradientPoints : public StaticComplexAnimator {
    e_OBJECT
protected:
    GradientPoints(PathBox * const parent);
public:
    void drawCanvasControls(SkCanvas *const canvas,
                            const CanvasMode mode,
                            const float invScale,
                            const bool ctrlPressed);

    void enable();
    void disable();

    void setColors(const QColor &startColor, const QColor &endColor);

    void setPositions(const QPointF &startPos, const QPointF &endPos);

    QPointF getStartPointAtRelFrame(const int relFrame);
    QPointF getEndPointAtRelFrame(const int relFrame);
    QPointF getStartPointAtRelFrameF(const qreal relFrame);
    QPointF getEndPointAtRelFrameF(const qreal relFrame);

    bool enabled() const {
        return mEnabled;
    }
private:
    bool mEnabled;

    qsptr<QPointFAnimator> mStartAnimator;
    qsptr<QPointFAnimator> mEndAnimator;

    stdsptr<GradientPoint> mStartPoint;
    stdsptr<GradientPoint> mEndPoint;

    PathBox* const mParent_k;
};

#endif // GRADIENTPOINTS_H
