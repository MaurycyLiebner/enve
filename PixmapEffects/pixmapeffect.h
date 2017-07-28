#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include "Animators/coloranimator.h"
#include "Animators/qpointfanimator.h"
#include "Properties/boolproperty.h"
#include <QObject>

namespace fmt_filters {
    struct image;
}

struct PixmapEffectRenderData {
    virtual void applyEffectsSk(const SkBitmap &imgPtr,
                                const fmt_filters::image &img,
                                const qreal &scale) = 0;
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

enum PixmapEffectType {
    EFFECT_BLUR,
    EFFECT_SHADOW,
    EFFECT_LINES,
    EFFECT_CIRCLES,
    EFFECT_SWIRL,
    EFFECT_DESATURATE,
    EFFECT_OIL,
    EFFECT_IMPLODE,
    EFFECT_COLORIZE
};

class EffectAnimators;

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

    virtual int saveToSql(QSqlQuery *query,
                              const int &boundingBoxSqlId);

    friend QDataStream & operator << (QDataStream & s,
                                      const PixmapEffect *ptr);
    friend QDataStream & operator >> (QDataStream & s,
                                      PixmapEffect *& ptr);

    virtual QMimeData *SWT_createMimeData() {
        return new PixmapEffectMimeData(this);
    }

    virtual void loadFromSql(const int &identifyingId) = 0;
    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;

    EffectAnimators *getParentEffectAnimators() {
        return mParentEffects;
    }

    void setParentEffectAnimators(EffectAnimators *parentEffects) {
        mParentEffects = parentEffects;
    }

    virtual PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
                                        const int &) { return NULL; }

    bool SWT_isPixmapEffect() { return true; }
public slots:
    void interrupt() {
        mInterrupted = true;
    }
protected:
    EffectAnimators *mParentEffects = NULL;
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

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &pixmapEffectId);

    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateBlurRadiusAnimatorFrom(QrealAnimator *source);

    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
                                    const int &relFrame);
private:
    QSharedPointer<BoolProperty> mHighQuality =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<QrealAnimator> mBlurRadius =
            (new QrealAnimator())->ref<QrealAnimator>();
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
};

class ShadowEffect : public PixmapEffect {
public:
    ShadowEffect(qreal radius = 10.);

    void applySk(const SkBitmap &imgPtr,
                 const fmt_filters::image &img,
                 qreal scale);

    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);

    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateTranslationAnimatorFrom(QPointFAnimator *source);
    void duplicateBlurRadiusAnimatorFrom(QrealAnimator *source);
    void duplicateColorAnimatorFrom(ColorAnimator *source);
    void duplicateOpacityAnimatorFrom(QrealAnimator *source);
    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame);
private:
//    QrealAnimator mScale;
    QSharedPointer<BoolProperty> mHighQuality =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<QrealAnimator> mBlurRadius =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mOpacity =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<ColorAnimator> mColor =
            (new ColorAnimator())->ref<ColorAnimator>();
    QSharedPointer<QPointFAnimator> mTranslation =
            (new QPointFAnimator())->ref<QPointFAnimator>();
};

class LinesEffect : public PixmapEffect
{
public:
    LinesEffect(qreal linesWidth = 5.,
                qreal linesDistance = 5.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateDistanceAnimatorFrom(QrealAnimator *source);
    void duplicateWidthAnimatorFrom(QrealAnimator *source);
private:
    QSharedPointer<QrealAnimator> mLinesDistance =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mLinesWidth =
            (new QrealAnimator())->ref<QrealAnimator>();
    bool mVertical = false;
};

class CirclesEffect : public PixmapEffect
{
public:
    CirclesEffect(qreal circlesRadius = 5.,
                  qreal circlesDistance = 5.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateDistanceAnimatorFrom(QrealAnimator *source);
    void duplicateRadiusAnimatorFrom(QrealAnimator *source);
private:
    QSharedPointer<QrealAnimator> mCirclesDistance =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mCirclesRadius =
            (new QrealAnimator())->ref<QrealAnimator>();
};

class SwirlEffect : public PixmapEffect {
public:
    SwirlEffect(qreal degrees = 45.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateDegreesAnimatorFrom(QrealAnimator *source);
private:
    QSharedPointer<QrealAnimator> mDegreesAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
};

class OilEffect : public PixmapEffect {
public:
    OilEffect(qreal radius = 2.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateRadiusAnimatorFrom(QrealAnimator *source);
private:
    QSharedPointer<QrealAnimator> mRadiusAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
};

class ImplodeEffect : public PixmapEffect {
public:
    ImplodeEffect(qreal radius = 10.);

    void apply(QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateFactorAnimatorFrom(QrealAnimator *source);
private:
    QSharedPointer<QrealAnimator> mFactorAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
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

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateInfluenceAnimatorFrom(QrealAnimator *source);
    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame);
private:
    QSharedPointer<QrealAnimator> mInfluenceAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
};

struct ColorizeEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    Color color;
};


class ColorizeEffect : public PixmapEffect {
public:
    ColorizeEffect();

    qreal getMargin() { return 0.; }

    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void loadFromSql(const int &identifyingId);
    Property *makeDuplicate();
    void makeDuplicate(Property *target);
    void duplicateInfluenceAnimatorFrom(ColorAnimator *source);
    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(
            const int &relFrame);
private:
    QSharedPointer<ColorAnimator> mColorAnimator =
            (new ColorAnimator())->ref<ColorAnimator>();
};

#endif // PIXMAPEFFECT_H
