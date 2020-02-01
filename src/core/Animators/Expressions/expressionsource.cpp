#include "expressionsource.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"

ExpressionSource::ExpressionSource(QrealAnimator * const parent) :
    ExpressionSourceBase(parent) {
    connect(parent, &QrealAnimator::prp_ancestorChanged,
            this, &ExpressionSource::updateSourcePath);
}

ExpressionValue::sptr ExpressionSource::sCreate(
        const QString &path, QrealAnimator * const parent) {
    const auto result = QSharedPointer<ExpressionSource>(
                new ExpressionSource(parent));
    result->setPath(path);
    return result;
}

qreal ExpressionSource::calculateValue(const qreal relFrame) const {
    const auto src = source();
    if(!src) return 1;
    const auto prnt = parent();
    const auto absFrame = prnt->prp_relFrameToAbsFrameF(relFrame);
    const auto sourceRelFrame = src->prp_absFrameToRelFrame(absFrame);
    return src->getEffectiveValue(sourceRelFrame);
}

FrameRange ExpressionSource::identicalRange(const qreal relFrame) const {
    const auto src = source();
    if(!src) return FrameRange::EMINMAX;
    const auto prnt = parent();
    const auto absFrame = prnt->prp_relFrameToAbsFrameF(relFrame);
    const auto sourceRelFrame = src->prp_absFrameToRelFrame(absFrame);
    const auto sourceAbsRange = src->prp_getIdenticalAbsRange(sourceRelFrame);
    const auto parentRelRange = prnt->prp_absRangeToRelRange(sourceAbsRange);
    return parentRelRange;
}

void ExpressionSource::setPath(const QString &path) {
    mPath = path;
    lookForSource();
}

void ExpressionSource::lookForSource() {
    const auto prnt = parent();
    const auto searchCtxt = prnt->getParent();
    QrealAnimator* newSource = nullptr;
    if(searchCtxt) {
        const auto objs = mPath.split('.');
        const auto found = searchCtxt->ca_findPropertyWithPathRec(0, objs);
        if(found != prnt) newSource = qobject_cast<QrealAnimator*>(found);
    }
    auto& conn = setSource(newSource);
    if(newSource) {
        conn << connect(newSource, &Property::prp_absFrameRangeChanged,
                        this, [this, prnt](const FrameRange& absRange) {
            const auto relRange = prnt->prp_absRangeToRelRange(absRange);
            emit relRangeChanged(relRange);
        });
        conn << connect(newSource, &QrealAnimator::effectiveValueChanged,
                        this, &ExpressionValue::updateValue);
        conn << connect(newSource, &QrealAnimator::prp_ancestorChanged,
                        this, &ExpressionSource::updateSourcePath);
    }
}

void ExpressionSource::updateSourcePath() {
    const auto src = source();
    if(!src) return;
    const auto prnt = parent();
    QStringList prntPath;
    prnt->prp_getFullPath(prntPath);
    QStringList srcPath;
    src->prp_getFullPath(srcPath);
    const int iMax = qMin(prntPath.count(), srcPath.count());
    for(int i = 0; i < iMax; i++) {
        const auto& iPrnt = prntPath.first();
        const auto& iSrc = srcPath.first();
        if(iPrnt == iSrc) {
            srcPath.removeFirst();
            prntPath.removeFirst();
        } else break;
    }
    mPath = srcPath.join('.');
}
