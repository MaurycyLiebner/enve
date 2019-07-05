#include "pixmapeffect.h"
#include "Animators/effectanimators.h"
#include <QDrag>

PixmapEffect::PixmapEffect(const QString &name,
                           const PixmapEffectType type) :
    StaticComplexAnimator(name), mType(type) {}

void PixmapEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mType), sizeof(PixmapEffectType));
}

void PixmapEffect::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mVisible), sizeof(bool));
}

void PixmapEffect::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    src->read(rcChar(&mVisible), sizeof(bool));
}

EffectAnimators *PixmapEffect::getParentEffectAnimators() {
    return static_cast<EffectAnimators*>(mParent_k.data());
}

QMarginsF PixmapEffect::getMarginAtRelFrame(const qreal relFrame) {
    Q_UNUSED(relFrame);
    return QMarginsF();
}

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
