#ifndef BRUSHSETTINGSANIMATOR_H
#define BRUSHSETTINGSANIMATOR_H
#include "Animators/staticcomplexanimator.h"
#include "Animators/qcubicsegment1danimator.h"
#include "GUI/BrushWidgets/simplebrushwrapper.h"

class BrushSettingsAnimator : public StaticComplexAnimator {
public:
    BrushSettingsAnimator();

    qCubicSegment1DAnimator * getWidthAnimator() const {
        return mWidthCurve.data();
    }

    qCubicSegment1DAnimator * getPressureAnimator() const {
        return mPressureCurve.data();
    }

    qCubicSegment1DAnimator * getSpacingAnimator() const {
        return mSpacingCurve.data();
    }

    qCubicSegment1DAnimator * getTimeAnimator() const {
        return mTimeCurve.data();
    }

    SimpleBrushWrapper* getBrush() const {
        return mBrush;
    }

    void setBrush(SimpleBrushWrapper * const brush) {
        mBrush = brush;
    }

    void setStrokeBrushWidthCurve(const qCubicSegment1D& curve) {
        mWidthCurve->setCurrentValue(curve);
    }

    void setStrokeBrushPressureCurve(const qCubicSegment1D& curve) {
        mPressureCurve->setCurrentValue(curve);
    }

    void setStrokeBrushSpacingCurve(const qCubicSegment1D& curve) {
        mSpacingCurve->setCurrentValue(curve);
    }

    void setStrokeBrushTimeCurve(const qCubicSegment1D& curve) {
        mTimeCurve->setCurrentValue(curve);
    }
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<qCubicSegment1DAnimator> mWidthCurve =
            SPtrCreate(qCubicSegment1DAnimator)("width");
    qsptr<qCubicSegment1DAnimator> mPressureCurve =
            SPtrCreate(qCubicSegment1DAnimator)("pressure");
    qsptr<qCubicSegment1DAnimator> mSpacingCurve =
            SPtrCreate(qCubicSegment1DAnimator)("spacing");
    qsptr<qCubicSegment1DAnimator> mTimeCurve =
            SPtrCreate(qCubicSegment1DAnimator)("time");

    stdptr<SimpleBrushWrapper> mBrush;
};

#endif // BRUSHSETTINGSANIMATOR_H
