#include "pixmapeffect.h"
#include "Animators/effectanimators.h"
#include <QDrag>

PixmapEffectRenderData::~PixmapEffectRenderData() {}

PixmapEffect::PixmapEffect(const QString &name,
                           const PixmapEffectType &type) :
    ComplexAnimator(name) {
    mType = type;
}

bool PixmapEffect::interrupted() {
    if(mInterrupted) {
        mInterrupted = false;
        return true;
    }
    return false;
}

qreal PixmapEffect::getMargin() { return 0.; }

qreal PixmapEffect::getMarginAtRelFrame(const int &) { return 0.; }

void PixmapEffect::prp_startDragging() {
    QMimeData *mimeData = new PixmapEffectMimeData(this);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

QMimeData *PixmapEffect::SWT_createMimeData() {
    return new PixmapEffectMimeData(this);
}

EffectAnimators *PixmapEffect::getParentEffectAnimators() {
    return mParentEffects;
}

void PixmapEffect::setParentEffectAnimators(EffectAnimators *parentEffects) {
    mParentEffects = parentEffects;
}

bool PixmapEffect::SWT_isPixmapEffect() { return true; }

void PixmapEffect::switchVisible() {
    setVisible(!mVisible);
}

void PixmapEffect::setVisible(const bool &visible) {
    if(visible == mVisible) return;
    mVisible = visible;
    prp_updateInfluenceRangeAfterChanged();
}

const bool &PixmapEffect::isVisible() {
    return mVisible;
}

void PixmapEffect::interrupt() {
    mInterrupted = true;
}
