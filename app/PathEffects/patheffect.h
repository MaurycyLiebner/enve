#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"
#include "Properties/boolproperty.h"
#include "Properties/comboboxproperty.h"
#include "Animators/intanimator.h"
#include "skiaincludes.h"
#include "Properties/boxtargetproperty.h"
#include "smartPointers/sharedpointerdefs.h"
class PathBox;
class PathEffectAnimators;
enum PathEffectType : short {
    DISPLACE_PATH_EFFECT,
    DASH_PATH_EFFECT,
    DUPLICATE_PATH_EFFECT,
    SOLIDIFY_PATH_EFFECT,
    SUM_PATH_EFFECT,
    GROUP_SUM_PATH_EFFECT,
    LENGTH_PATH_EFFECT
};
class PathEffect;

typedef PropertyMimeData<PathEffect,
    InternalMimeData::PATH_EFFECT> PathEffectMimeData;

class PathEffect : public ComplexAnimator {
    Q_OBJECT
public:
    PathEffect(const QString& name,
               const PathEffectType &type,
               const bool &outlinePathEffect);

    const PathEffectType &getEffectType();

    virtual void filterPathForRelFrame(const int &,
                                       const SkPath &,
                                       SkPath *,
                                       const qreal &,
                                       const bool &) = 0;

    virtual void filterPathForRelFrameF(const qreal &,
                                       const SkPath &,
                                       SkPath *,
                                       const bool &) = 0;
    virtual void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    bool applyBeforeThickness();
    void setParentEffectAnimators(PathEffectAnimators *parent);

    PathEffectAnimators *getParentEffectAnimators();

    QMimeData *SWT_createMimeData();

    bool SWT_isPathEffect();

    void setIsOutlineEffect(const bool &bT);

    void switchVisible();

    void setVisible(const bool &bT);

    const bool &isVisible();

    virtual bool hasReasonsNotToApplyUglyTransform();
    void prp_startDragging();
protected:
    bool mVisible = true;
    bool mOutlineEffect = false;
    PathEffectType mPathEffectType;
    qsptr<BoolProperty> mApplyBeforeThickness;
    qptr<PathEffectAnimators> mParentEffectAnimators;
};

#endif // PATHEFFECT_H
