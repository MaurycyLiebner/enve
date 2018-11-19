#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include <QObject>
#include <QSharedPointer>
#include "skiaincludes.h"
#include "Colors/color.h"
#include "Animators/complexanimator.h"
#include <QMimeData>
#include <QPointF>
class EffectAnimators;
class QIODevice;

#include "sharedpointerdefs.h"

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

class PixmapEffectMimeData : public QMimeData {
    Q_OBJECT
public:
    PixmapEffectMimeData(PixmapEffect *target) : QMimeData() {
        mPixmapEffect = target;
    }

    PixmapEffect *getPixmapEffect() {
        return mPixmapEffect;
    }

    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "pixmapeffect") return true;
        return false;
    }

private:
    PixmapEffect *mPixmapEffect;
};

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
public:
    PixmapEffect(const PixmapEffectType &type);

    bool interrupted() {
        if(mInterrupted) {
            mInterrupted = false;
            return true;
        }
        return false;
    }

    virtual qreal getMargin() { return 0.; }
    virtual qreal getMarginAtRelFrame(const int &) { return 0.; }


    void prp_startDragging();

    friend QDataStream & operator << (QDataStream & s,
                                      const PixmapEffect *ptr);
    friend QDataStream & operator >> (QDataStream & s,
                                      PixmapEffect *& ptr);

    QMimeData *SWT_createMimeData() {
        return new PixmapEffectMimeData(this);
    }

    EffectAnimators *getParentEffectAnimators() {
        return mParentEffects;
    }

    void setParentEffectAnimators(EffectAnimators *parentEffects) {
        mParentEffects = parentEffects;
    }

    virtual PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& data) = 0;

    bool SWT_isPixmapEffect() { return true; }

    virtual void writeProperty(QIODevice *target);
    void switchVisible() {
        setVisible(!mVisible);
    }

    void setVisible(const bool &visible) {
        if(visible == mVisible) return;
        mVisible = visible;
        prp_updateInfluenceRangeAfterChanged();
    }

    const bool &isVisible() {
        return mVisible;
    }
    void readProperty(QIODevice *target);
public slots:
    void interrupt() {
        mInterrupted = true;
    }
protected:
    bool mInterrupted = false;
    bool mVisible = true;
    EffectAnimators *mParentEffects = nullptr;
    PixmapEffectType mType; 
};

struct BlurEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys;
    bool highQuality;
    qreal blurRadius;

    static BlurEffectRenderDataSPtr createBlurEffectRenderData() {
        return (new BlurEffectRenderData)->ref<BlurEffectRenderData>();
    }
private:
    BlurEffectRenderData() {}
};

class BlurEffect : public PixmapEffect {
public:
    BlurEffect(qreal radius = 10.);

    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);


    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
private:
    BoolPropertyQSPtr mHighQuality;
    QrealAnimatorQSPtr mBlurRadius;
};

struct ShadowEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys;
    bool highQuality;
    qreal blurRadius;
    Color color;
    QPointF translation;
    qreal opacity;

    static ShadowEffectRenderDataSPtr createShadowEffectRenderData() {
        return (new ShadowEffectRenderData)->ref<ShadowEffectRenderData>();
    }
private:
    ShadowEffectRenderData() {}
};

class ShadowEffect : public PixmapEffect {
public:
    ShadowEffect(qreal radius = 10.);

    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
private:
//    QrealAnimator mScale;
    BoolPropertyQSPtr mHighQuality;
    QrealAnimatorQSPtr mBlurRadius;
    QrealAnimatorQSPtr mOpacity;
    ColorAnimatorQSPtr mColor;
    QPointFAnimatorQSPtr mTranslation;
};

struct LinesEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool vertical = false;
    qreal linesDistance;
    qreal linesWidth;


    static LinesEffectRenderDataSPtr createLinesEffectRenderData() {
        return (new LinesEffectRenderData)->ref<LinesEffectRenderData>();
    }
private:
    LinesEffectRenderData() {}
};

class LinesEffect : public PixmapEffect {
public:
    LinesEffect(qreal linesWidth = 5.,
                qreal linesDistance = 5.);
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
private:
    bool mVertical = false;
    QrealAnimatorQSPtr mLinesDistance;
    QrealAnimatorQSPtr mLinesWidth;
};

struct CirclesEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal circlesDistance;
    qreal circlesRadius;


    static CirclesEffectRenderDataSPtr createCirclesEffectRenderData() {
        return (new CirclesEffectRenderData)->ref<CirclesEffectRenderData>();
    }
private:
    CirclesEffectRenderData() {}
};

class CirclesEffect : public PixmapEffect {
public:
    CirclesEffect(qreal circlesRadius = 5.,
                  qreal circlesDistance = 5.);

    qreal getMargin() { return 0.; }
    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
private:
    QrealAnimatorQSPtr mCirclesDistance;
    QrealAnimatorQSPtr mCirclesRadius;
};

struct SwirlEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal degrees;


    static SwirlEffectRenderDataSPtr createSwirlEffectRenderData() {
        return (new SwirlEffectRenderData)->ref<SwirlEffectRenderData>();
    }
private:
    SwirlEffectRenderData() {}
};

class SwirlEffect : public PixmapEffect {
public:
    SwirlEffect(qreal degrees = 45.);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
private:
    QrealAnimatorQSPtr mDegreesAnimator;
};

struct OilEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal radius;


    static OilEffectRenderDataSPtr createOilEffectRenderData() {
        return (new OilEffectRenderData)->ref<OilEffectRenderData>();
    }
private:
    OilEffectRenderData() {}
};

class OilEffect : public PixmapEffect {
public:
    OilEffect(qreal radius = 2.);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
private:
    QrealAnimatorQSPtr mRadiusAnimator;
};

struct ImplodeEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal factor;


    static ImplodeEffectRenderDataSPtr createImplodeEffectRenderData() {
        return (new ImplodeEffectRenderData)->ref<ImplodeEffectRenderData>();
    }
private:
    ImplodeEffectRenderData() {}
};

class ImplodeEffect : public PixmapEffect {
public:
    ImplodeEffect(qreal radius = 10.);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
private:
    QrealAnimatorQSPtr mFactorAnimator;
};

struct DesaturateEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal influence;


    static DesaturateEffectRenderDataSPtr createDesaturateEffectRenderData() {
        return (new DesaturateEffectRenderData)->ref<DesaturateEffectRenderData>();
    }
private:
    DesaturateEffectRenderData() {}
};

class DesaturateEffect : public PixmapEffect {
public:
    DesaturateEffect(qreal influence = .5);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mInfluenceAnimator;
};

struct ColorizeEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal hue;
    qreal saturation;
    qreal lightness;
    qreal alpha;

    static ColorizeEffectRenderDataSPtr createColorizeEffectRenderData() {
        return (new ColorizeEffectRenderData)->ref<ColorizeEffectRenderData>();
    }
private:
    ColorizeEffectRenderData() {}
};


class ColorizeEffect : public PixmapEffect {
public:
    ColorizeEffect();

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mHueAnimator;
    QrealAnimatorQSPtr mSaturationAnimator;
    QrealAnimatorQSPtr mLightnessAnimator;
    QrealAnimatorQSPtr mAlphaAnimator;
};

struct ReplaceColorEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    int redR;
    int greenR;
    int blueR;
    int alphaR;
    int redT;
    int greenT;
    int blueT;
    int alphaT;
    int tolerance;
    qreal smoothness;

    static ReplaceColorEffectRenderDataSPtr createReplaceColorEffectRenderData() {
        return (new ReplaceColorEffectRenderData)->ref<ReplaceColorEffectRenderData>();
    }
private:
    ReplaceColorEffectRenderData() {}
};

class ReplaceColorEffect : public PixmapEffect {
public:
    ReplaceColorEffect();

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
private:
    ColorAnimatorQSPtr mFromColor;
    ColorAnimatorQSPtr mToColor;

    QrealAnimatorQSPtr mToleranceAnimator;
    QrealAnimatorQSPtr mSmoothnessAnimator;
};

struct ContrastEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys = false;
    qreal contrast;

    static ContrastEffectRenderDataSPtr createContrastEffectRenderData() {
        return (new ContrastEffectRenderData)->ref<ContrastEffectRenderData>();
    }
private:
    ContrastEffectRenderData() {}
};

class ContrastEffect : public PixmapEffect {
public:
    ContrastEffect(qreal contrast = .0);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mContrastAnimator;
};

struct BrightnessEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys = false;
    qreal brightness;

    static BrightnessEffectRenderDataSPtr createBrightnessEffectRenderData() {
        return (new BrightnessEffectRenderData)->ref<BrightnessEffectRenderData>();
    }
private:
    BrightnessEffectRenderData() {}
};

class BrightnessEffect : public PixmapEffect {
public:
    BrightnessEffect(qreal brightness = .0);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& );
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mBrightnessAnimator;
};

struct SampledMotionBlurEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal numberSamples;
    qreal opacity;
    BoundingBoxRenderDataSPtr boxData;
    QList<BoundingBoxRenderDataSPtr> samples;


    static SampledMotionBlurEffectRenderDataSPtr createSampledMotionBlurEffectRenderData() {
        return (new SampledMotionBlurEffectRenderData)->ref<SampledMotionBlurEffectRenderData>();
    }
private:
    SampledMotionBlurEffectRenderData() {}
};

class SampledMotionBlurEffect : public PixmapEffect {
public:
    SampledMotionBlurEffect(BoundingBox *box = nullptr);

    qreal getMargin() { return 0.; }

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    void setParentBox(BoundingBox *box) {
        mParentBox = box;
    }

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& data);
    void prp_setAbsFrame(const int &frame);
private:
    void getParentBoxFirstLastMarginAjusted(int *firstT, int *lastT,
                                            const int &relFrame);
    BoundingBox *mParentBox = nullptr;
    QrealAnimatorQSPtr mOpacity;
    QrealAnimatorQSPtr mNumberSamples;
    QrealAnimatorQSPtr mFrameStep;
};
#endif // PIXMAPEFFECT_H
