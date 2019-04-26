#include "gradient.h"
#include "Animators/coloranimator.h"
#include "PropertyUpdaters/gradientupdater.h"
#include "Boxes/pathbox.h"

Gradient::Gradient() : ComplexAnimator("gradient") {
    prp_setOwnUpdater(SPtrCreate(GradientUpdater)(this));
}

Gradient::Gradient(const QColor &color1, const QColor &color2) :
    Gradient() {
    addColorToList(color1);
    addColorToList(color2);
    updateQGradientStops(Animator::USER_CHANGE);
}

bool Gradient::isEmpty() const {
    return mColors.isEmpty();
}

void Gradient::prp_startTransform() {
    //savedColors = colors;
}

void Gradient::addColorToList(const QColor &color) {
    auto newColorAnimator = SPtrCreate(ColorAnimator)();
    newColorAnimator->qra_setCurrentValue(color);
    addColorToList(newColorAnimator);
}

void Gradient::addColorToList(const qsptr<ColorAnimator>& newColorAnimator) {
    mColors << newColorAnimator;

    ca_addChildAnimator(newColorAnimator);
}

QColor Gradient::getCurrentColorAt(const int &id) {
    return mColors.at(id)->getCurrentColor();
}

ColorAnimator *Gradient::getColorAnimatorAt(const int &id) {
    return mColors.at(id).get();
}

int Gradient::getColorCount() {
    return mColors.length();
}

QColor Gradient::getLastQGradientStopQColor() {
    return mQGradientStops.last().second;
}

QColor Gradient::getFirstQGradientStopQColor() {
    return mQGradientStops.first().second;
}

QGradientStops Gradient::getQGradientStops() {
    return mQGradientStops;
}

void Gradient::swapColors(const int &id1, const int &id2) {
    ca_swapChildAnimators(mColors.at(id1).get(),
                          mColors.at(id2).get());
    mColors.swap(id1, id2);
    updateQGradientStops(Animator::USER_CHANGE);
    prp_updateInfluenceRangeAfterChanged();
}

void Gradient::removeColor(const int &id) {
    removeColor(mColors.at(id));
}

void Gradient::removeColor(const qsptr<ColorAnimator>& color) {
    ca_removeChildAnimator(color);
    emit resetGradientWidgetColorIdIfEquals(this, mColors.indexOf(color));
    mColors.removeOne(color);
    updateQGradientStops(Animator::USER_CHANGE);
    prp_updateInfluenceRangeAfterChanged();
}

void Gradient::addColor(const QColor &color) {
    addColorToList(color);
    updateQGradientStops(Animator::USER_CHANGE);
    prp_updateInfluenceRangeAfterChanged();
}

void Gradient::replaceColor(const int &id, const QColor &color) {
    mColors.at(id)->qra_setCurrentValue(color);
    updateQGradientStops(Animator::USER_CHANGE);
    prp_updateInfluenceRangeAfterChanged();
}

void Gradient::addPath(PathBox * const path) {
    mAffectedPaths << path;
}

void Gradient::removePath(PathBox * const path) {
    mAffectedPaths.removeOne(path);
}

bool Gradient::affectsPaths() {
    return !mAffectedPaths.isEmpty();
}

void Gradient::startColorIdTransform(const int& id) {
    if(mColors.count() <= id || id < 0) return;
    mColors.at(id)->prp_startTransform();
}

QGradientStops Gradient::getQGradientStopsAtAbsFrame(const qreal &absFrame) {
    QGradientStops stops;
    const qreal inc = 1./(mColors.length() - 1);
    qreal cPos = 0.;
    for(int i = 0; i < mColors.length(); i++) {
        stops.append(QPair<qreal, QColor>(clamp(cPos, 0, 1),
                     mColors.at(i)->getColorAtRelFrame(absFrame)) );
        cPos += inc;
    }
    return stops;
}

void Gradient::updateQGradientStops(const Animator::UpdateReason& reason) {
    mQGradientStops.clear();
    const qreal inc = 1./(mColors.length() - 1);
    qreal cPos = 0;
    for(int i = 0; i < mColors.length(); i++) {
        mQGradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0, 1),
                                    mColors.at(i)->getCurrentColor()) );
        cPos += inc;
    }
    for(const auto& path : mAffectedPaths) {
        path->updateDrawGradients();
        path->planScheduleUpdate(reason);
    }
}


int Gradient::getLoadId() {
    return mLoadId;
}

void Gradient::setLoadId(const int &id) {
    mLoadId = id;
}
