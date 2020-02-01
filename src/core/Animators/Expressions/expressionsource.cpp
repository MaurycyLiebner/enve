#include "expressionsource.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"

ExpressionSource::ExpressionSource(QrealAnimator * const parent) :
    mParent(parent) {}

ExpressionValue::sptr ExpressionSource::sCreate(
        const QString &path, QrealAnimator * const parent) {
    const auto result = QSharedPointer<ExpressionSource>(
                new ExpressionSource(parent));
    result->setPath(path);
    return result;
}

qreal ExpressionSource::calculateValue(const qreal relFrame) const {
    if(!mSource) return 1;
    const auto absFrame = mParent->prp_relFrameToAbsFrameF(relFrame);
    const auto sourceRelFrame = mSource->prp_absFrameToRelFrame(absFrame);
    return mSource->getEffectiveValue(sourceRelFrame);
}

bool ExpressionSource::isValid() const
{ return mSource; }

FrameRange ExpressionSource::identicalRange(const qreal relFrame) const {
    if(!mSource) return FrameRange::EMINMAX;
    const auto absFrame = mParent->prp_relFrameToAbsFrameF(relFrame);
    const auto sourceRelFrame = mSource->prp_absFrameToRelFrame(absFrame);
    const auto sourceAbsRange = mSource->prp_getIdenticalAbsRange(sourceRelFrame);
    const auto parentRelRange = mParent->prp_absRangeToRelRange(sourceAbsRange);
    return parentRelRange;
}

void ExpressionSource::setPath(const QString &path) {
    mPath = path.split('.');
    lookForSource();
}

void ExpressionSource::lookForSource() {
    const auto searchCtxt = mParent->getParent();
    QrealAnimator* newSource = nullptr;
    if(searchCtxt) {
        const auto found = searchCtxt->ca_findPropertyWithPathRecBothWays(0, mPath);
        if(found != mParent) newSource = qobject_cast<QrealAnimator*>(found);
    }
    auto& conn = mSource.assign(newSource);
    if(newSource) {
        conn << connect(newSource, &Property::prp_absFrameRangeChanged,
                        this, [this](const FrameRange& absRange) {
            const auto relRange = mParent->prp_absRangeToRelRange(absRange);
            emit relRangeChanged(relRange);
        });
        conn << connect(newSource, &QrealAnimator::effectiveValueChanged,
                        this, &ExpressionValue::updateValue);
    }
}
