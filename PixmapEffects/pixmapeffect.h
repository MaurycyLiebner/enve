#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include "fmt_filters.h"
#include "Animators/coloranimator.h"
#include "Animators/qpointfanimator.h"
#include <QObject>

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
    EFFECT_CIRCLES
};

class PixmapEffect : public ComplexAnimator
{
    Q_OBJECT
public:
    PixmapEffect();
    virtual void apply(BoundingBox *,
                       QImage *,
                       const fmt_filters::image &,
                       qreal) {}

    virtual qreal getMargin() { return 0.; }

    bool interrupted() {
        if(mInterrupted) {
            mInterrupted = false;
            return true;
        }
        return false;
    }

    void startDragging();

    int saveToSql(QSqlQuery *query,
                  const int &boundingBoxSqlId,
                  const PixmapEffectType &type);
    static PixmapEffect *loadFromSql(int pixmapEffectId,
                                     PixmapEffectType typeT);
    virtual void saveToSql(QSqlQuery *,
                           const int &) {}

    friend QDataStream & operator << (QDataStream & s, const PixmapEffect *ptr);
    friend QDataStream & operator >> (QDataStream & s, PixmapEffect *& ptr);

    virtual QMimeData *SWT_createMimeData() {
        return new PixmapEffectMimeData(this);
    }

    SWT_Type SWT_getType() { return SWT_PixmapEffect; }

public slots:
    void interrupt() {
        mInterrupted = true;
    }
protected:
    bool mInterrupted = false;
};

class BlurEffect : public PixmapEffect
{
public:
    BlurEffect(qreal radius = 10.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin();

    void loadBlurEffectFromSql(int pixmapEffectId);

    void saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);

    QrealAnimator *makeDuplicate();
    void makeDuplicate(QrealAnimator *target);
    void duplicateBlurRadiusAnimatorFrom(QrealAnimator *source);
private:
    QrealAnimator mBlurRadius;
};

class ShadowEffect : public PixmapEffect
{
public:
    ShadowEffect(qreal radius = 10.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin();

    QrealAnimator *makeDuplicate();
    void makeDuplicate(QrealAnimator *target);
    void duplicateTranslationAnimatorFrom(QPointFAnimator *source);
    void duplicateBlurRadiusAnimatorFrom(QrealAnimator *source);
    void duplicateColorAnimatorFrom(ColorAnimator *source);
    void duplicateOpacityAnimatorFrom(QrealAnimator *source);
private:
//    QrealAnimator mScale;
    QPointFAnimator mTranslation;
    ColorAnimator mColor;
    QrealAnimator mBlurRadius;
    QrealAnimator mOpacity;
};

class LinesEffect : public PixmapEffect
{
public:
    LinesEffect(qreal linesWidth = 5.,
                qreal linesDistance = 5.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mLinesDistance;
    QrealAnimator mLinesWidth;
    bool mVertical = false;
};

class CirclesEffect : public PixmapEffect
{
public:
    CirclesEffect(qreal circlesRadius = 5.,
                  qreal circlesDistance = 5.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mCirclesDistance;
    QrealAnimator mCirclesRadius;
};

class SwirlEffect : public PixmapEffect {
public:
    SwirlEffect(qreal degrees = 45.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mDegreesAnimator;
};

class OilEffect : public PixmapEffect {
public:
    OilEffect(qreal radius = 2.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mRadiusAnimator;
};

class ImplodeEffect : public PixmapEffect {
public:
    ImplodeEffect(qreal radius = 10.);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mFactorAnimator;
};

class DesaturateEffect : public PixmapEffect {
public:
    DesaturateEffect(qreal influence = .5);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mInfluenceAnimator;
};

#endif // PIXMAPEFFECT_H
