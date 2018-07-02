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
class QPointFAnimator;
class QrealAnimator;
class ColorAnimator;
class BoolProperty;
struct BoundingBoxRenderData;
typedef std::shared_ptr<BoundingBoxRenderData> BoundingBoxRenderDataSPtr;
typedef QSharedPointer<QPointFAnimator> QPointFAnimatorQSPtr;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;
typedef QSharedPointer<ColorAnimator> ColorAnimatorQSPtr;
typedef QSharedPointer<BoolProperty> BoolPropertyQSPtr;

namespace fmt_filters {
    struct image;
}

struct PixmapEffectRenderData {
    virtual void applyEffectsSk(const SkBitmap &imgPtr,
                                const fmt_filters::image &img,
                                const qreal &scale) = 0;
    virtual ~PixmapEffectRenderData() {}
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
    virtual void apply(QImage *,
                       const fmt_filters::image &,
                       qreal) {}
    virtual void applySk(const SkBitmap &,
                         const fmt_filters::image &,
                         qreal) {}

    virtual qreal getMargin() { return 0.; }
    virtual qreal getMarginAtRelFrame(const int &) { return 0.; }
    bool interrupted() {
        if(mInterrupted) {
            mInterrupted = false;
            return true;
        }
        return false;
    }

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

    virtual PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
                                        const int &,
                                        BoundingBoxRenderData *) { return nullptr; }
    virtual PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrameF(
                                        const qreal &relFrame,
                                        BoundingBoxRenderData *data) { /*return nullptr; }*/
        return getPixmapEffectRenderDataForRelFrame(relFrame, data);
    }

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
    bool mVisible = true;
    EffectAnimators *mParentEffects = nullptr;
    PixmapEffectType mType;
    bool mInterrupted = false;
};

struct BlurEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys;
    bool highQuality;
    qreal blurRadius;
};

class BlurEffect : public PixmapEffect {
public:
    BlurEffect(qreal radius = 10.);

    void applySk(const SkBitmap &imgPtr,
                 const fmt_filters::image &img,
                 qreal scale);
    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);


    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
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
};

class ShadowEffect : public PixmapEffect {
public:
    ShadowEffect(qreal radius = 10.);

    void applySk(const SkBitmap &imgPtr,
                 const fmt_filters::image &img,
                 qreal scale);

    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
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

class LinesEffect : public PixmapEffect {
public:
    LinesEffect(qreal linesWidth = 5.,
                qreal linesDistance = 5.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

private:
    QrealAnimatorQSPtr mLinesDistance;
    QrealAnimatorQSPtr mLinesWidth;
    bool mVertical = false;
};

class CirclesEffect : public PixmapEffect {
public:
    CirclesEffect(qreal circlesRadius = 5.,
                  qreal circlesDistance = 5.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

private:
    QrealAnimatorQSPtr mCirclesDistance;
    QrealAnimatorQSPtr mCirclesRadius;
};

class SwirlEffect : public PixmapEffect {
public:
    SwirlEffect(qreal degrees = 45.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

private:
    QrealAnimatorQSPtr mDegreesAnimator;
};

class OilEffect : public PixmapEffect {
public:
    OilEffect(qreal radius = 2.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }


private:
    QrealAnimatorQSPtr mRadiusAnimator;
};

class ImplodeEffect : public PixmapEffect {
public:
    ImplodeEffect(qreal radius = 10.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }


private:
    QrealAnimatorQSPtr mFactorAnimator;
};

struct DesaturateEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal influence;
};

class DesaturateEffect : public PixmapEffect {
public:
    DesaturateEffect(qreal influence = .5);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
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
};


class ColorizeEffect : public PixmapEffect {
public:
    ColorizeEffect();

    qreal getMargin() { return 0.; }

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
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
    qreal smoothness;
    int tolerance;
};

class ReplaceColorEffect : public PixmapEffect {
public:
    ReplaceColorEffect();

    qreal getMargin() { return 0.; }

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
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

    qreal contrast;
    bool hasKeys = false;
};

class ContrastEffect : public PixmapEffect {
public:
    ContrastEffect(qreal contrast = .0);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mContrastAnimator;
};

struct BrightnessEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal brightness;
    bool hasKeys = false;
};

class BrightnessEffect : public PixmapEffect {
public:
    BrightnessEffect(qreal brightness = .0);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
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
    BoundingBoxRenderData *boxData;
    QList<BoundingBoxRenderDataSPtr> samples;
};

class SampledMotionBlurEffect : public PixmapEffect {
public:
    SampledMotionBlurEffect(BoundingBox *box = nullptr);

    qreal getMargin() { return 0.; }

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame,
            BoundingBoxRenderData *data);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    void setParentBox(BoundingBox *box) {
        mParentBox = box;
    }

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrameF(
                                        const qreal &relFrame,
            BoundingBoxRenderData *data);
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
