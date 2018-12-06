#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include "skiaincludes.h"
#include "Animators/complexanimator.h"
#include <QMimeData>
#include <QPointF>
class EffectAnimators;
class QIODevice;
class BoundingBoxRenderData;

namespace fmt_filters {
    struct image;
}

struct PixmapEffectRenderData : public StdSelfRef {
    virtual void applyEffectsSk(const SkBitmap &imgPtr,
                                const fmt_filters::image &img,
                                const qreal &scale) = 0;
    virtual ~PixmapEffectRenderData();
};

class PixmapEffect;

typedef PropertyMimeData<PixmapEffect,
    InternalMimeData::PIXMAP_EFFECT> PixmapEffectMimeData;

enum PixmapEffectType : short {
    EFFECT_BLUR,
    EFFECT_SHADOW,
    EFFECT_LINES,
    EFFECT_CIRCLES,
    EFFECT_SWIRL,
    EFFECT_DESATURATE,
    EFFECT_OIL,
    EFFECT_IMPLODE,
    EFFECT_COLORIZE,
    EFFECT_REPLACE_COLOR,
    EFFECT_CONTRAST,
    EFFECT_BRIGHTNESS,
    EFFECT_BRUSH,
    EFFECT_MOTION_BLUR
};

class PixmapEffect : public ComplexAnimator {
    Q_OBJECT
    friend class SelfRef;
public:
    PixmapEffect(const QString& name, const PixmapEffectType &type);

    bool interrupted();

    virtual qreal getMargin();
    virtual qreal getMarginAtRelFrame(const int &);


    void prp_startDragging();

    friend QDataStream & operator << (QDataStream & s,
                                      const PixmapEffect *ptr);
    friend QDataStream & operator >> (QDataStream & s,
                                      PixmapEffect *& ptr);

    QMimeData *SWT_createMimeData();

    EffectAnimators *getParentEffectAnimators();

    void setParentEffectAnimators(EffectAnimators *parentEffects);

    virtual stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* data) = 0;

    bool SWT_isPixmapEffect();

    virtual void writeProperty(QIODevice *target);
    void switchVisible();

    void setVisible(const bool &visible);

    const bool &isVisible();
    void readProperty(QIODevice *target);
public slots:
    void interrupt();
protected:
    bool mInterrupted = false;
    bool mVisible = true;
    PixmapEffectType mType;
    qptr<EffectAnimators> mParentEffects;
};
#endif // PIXMAPEFFECT_H
