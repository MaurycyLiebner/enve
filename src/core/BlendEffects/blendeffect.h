#ifndef BLENDEFFECT_H
#define BLENDEFFECT_H

#include "Animators/staticcomplexanimator.h"

#include "Animators/intanimator.h"
#include "Properties/boxtargetproperty.h"

class PathBox;

class BlendEffect : public StaticComplexAnimator {
    Q_OBJECT
    e_OBJECT
protected:
    BlendEffect();
public:
    bool isPathValid() const;
    int zIndex(const qreal relFrame) const;
    SkPath clipPath(const qreal relFrame) const;
private:
    PathBox* clipPathSource() const;

    qsptr<IntAnimator> mZIndex;
    qsptr<BoxTargetProperty> mClipPath;
};

#endif // BLENDEFFECT_H
