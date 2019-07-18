#ifndef PAINTSETTINGSAPPLIER_H
#define PAINTSETTINGSAPPLIER_H
#include "smartPointers/sharedpointerdefs.h"
#include "colorsetting.h"
#include "Segments/qcubicsegment1d.h"
#include "GUI/BrushWidgets/brushwidget.h"
#include <QPainter>
#include "Boxes/pathbox.h"
#include "Animators/gradient.h"
#include "Animators/outlinesettingsanimator.h"
class ColorAnimator;
class Gradient;
class PathBox;
enum PaintType : short;
class PaintSettingsAnimator;
class FillSettingsAnimator;
class OutlineSettingsAnimator;

class PaintSetting {
public:
    enum Target { FILL, OUTLINE };
    virtual ~PaintSetting() {}

    void apply(PathBox * const target) const {
        applyToPS(targetPaintSettings(target));
    }
protected:
    PaintSetting(const Target& target) : mTarget(target) {}
    virtual void applyToPS(PaintSettingsAnimator * const target) const = 0;
private:
    PaintSettingsAnimator * targetPaintSettings(PathBox * const target) const {
        if(mTarget == FILL) return target->getFillSettings();
        return target->getStrokeSettings();
    }
    const Target mTarget;
};

class ColorPaintSetting : public PaintSetting {
public:
    ColorPaintSetting(const Target& target,
                      const ColorSettingApplier& colorSettings) :
        PaintSetting(target), mColorSetting(colorSettings) {}
protected:
    void applyToPS(PaintSettingsAnimator * const target) const {
        mColorSetting.apply(target->getColorAnimator());
    }
private:
    const ColorSettingApplier mColorSetting;
};

class GradientPaintSetting : public PaintSetting {
public:
    GradientPaintSetting(const Target& target,
                         Gradient * const gradient) :
        PaintSetting(target), mGradient(gradient) {}
protected:
    void applyToPS(PaintSettingsAnimator * const target) const {
        target->setGradient(mGradient);
    }
private:
    Gradient * const mGradient;
};

class GradientPtsPosSetting : public PaintSetting {
public:
    GradientPtsPosSetting(const Target& target,
                          const QPointF& pt1, const QPointF& pt2) :
        PaintSetting(target), mPt1(pt1), mPt2(pt2) {}
protected:
    void applyToPS(PaintSettingsAnimator * const target) const {
        target->setGradientPointsPos(mPt1, mPt2);
    }
private:
    const QPointF mPt1;
    const QPointF mPt2;
};

class GradientTypePaintSetting : public PaintSetting {
public:
    GradientTypePaintSetting(const Target& target,
                             const GradientType& type) :
        PaintSetting(target), mGradientType(type) {}
protected:
    void applyToPS(PaintSettingsAnimator * const target) const {
        target->setGradientType(mGradientType);
    }
private:
    const GradientType mGradientType;
};

class PaintTypePaintSetting : public PaintSetting {
public:
    PaintTypePaintSetting(const Target& target,
                          const PaintType& type) :
        PaintSetting(target), mPaintType(type) {}
protected:
    void applyToPS(PaintSettingsAnimator * const target) const {
        target->setPaintType(mPaintType);
    }
private:
    const PaintType mPaintType;
};

class OutlineWidthPaintSetting : public PaintSetting {
public:
    OutlineWidthPaintSetting(const qreal width) :
        PaintSetting(OUTLINE), mWidth(width) {}
protected:
    void applyToPS(PaintSettingsAnimator * const target) const {
        static_cast<OutlineSettingsAnimator*>(target)->setCurrentStrokeWidth(mWidth);
    }
private:
    const qreal mWidth;
};

class PaintSettingsApplier {
public:
    inline PaintSettingsApplier &operator<< (const stdsptr<PaintSetting> &t)
    { mSettings << t; return *this; }

    void apply(PathBox * const target) const {
        for(const auto& setting : mSettings)
            setting->apply(target);
    }
private:
    QList<stdsptr<PaintSetting>> mSettings;
};

#endif // PAINTSETTINGSAPPLIER_H
