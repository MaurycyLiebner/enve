#include "pixmapeffect.h"
#include "Animators/effectanimators.h"
#include <QDrag>

PixmapEffect::PixmapEffect(const QString &name,
                           const PixmapEffectType type) :
    ComplexAnimator(name), mType(type) {}

EffectAnimators *PixmapEffect::getParentEffectAnimators() {
    return static_cast<EffectAnimators*>(mParent.data());
}

qreal PixmapEffect::getMargin() { return 0.; }

qreal PixmapEffect::getMarginAtRelFrame(const int ) { return 0.; }

void PixmapEffect::prp_startDragging() {
    QMimeData *mimeData = new PixmapEffectMimeData(this);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

QMimeData *PixmapEffect::SWT_createMimeData() {
    return new PixmapEffectMimeData(this);
}

bool PixmapEffect::SWT_isPixmapEffect() const { return true; }

void PixmapEffect::switchVisible() {
    setVisible(!mVisible);
}

void PixmapEffect::setVisible(const bool visible) {
    if(visible == mVisible) return;
    mVisible = visible;
    prp_afterWholeInfluenceRangeChanged();
}

bool PixmapEffect::isVisible() const {
    return mVisible;
}
