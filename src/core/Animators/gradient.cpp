// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "gradient.h"
#include "Animators/coloranimator.h"
#include "Boxes/pathbox.h"

int Gradient::sNextDocumnetId = 0;

Gradient::Gradient() : ComplexAnimator("gradient"),
    mDocumentId(sNextDocumnetId++) {
    connect(this, &Property::prp_currentFrameChanged,
            this, &Gradient::updateQGradientStops);
}

Gradient::Gradient(const QColor &color1, const QColor &color2) :
    Gradient() {
    addColorToList(color1);
    addColorToList(color2);
}

void Gradient::write(const int id, eWriteStream& dst) {
    mReadWriteId = id;
    dst << id;
    writeProperty(dst);
}

int Gradient::read(eReadStream& src) {
    src >> mReadWriteId;
    readProperty(src);
    return mReadWriteId;
}

void Gradient::writeProperty(eWriteStream& dst) const {
    const int nColors = mColors.count();
    dst << nColors;
    for(const auto& color : mColors)
        color->writeProperty(dst);
}

void Gradient::readProperty(eReadStream& src) {
    int nColors;
    src >> nColors;
    for(int i = 0; i < nColors; i++) {
        const auto colorAnim = enve::make_shared<ColorAnimator>();
        colorAnim->readProperty(src);
        addColorToList(colorAnim);
    }
    updateQGradientStops(UpdateReason::userChange);
}

bool Gradient::isEmpty() const {
    return mColors.isEmpty();
}

void Gradient::prp_startTransform() {
    //savedColors = colors;
}

void Gradient::prp_setInheritedFrameShift(const int shift,
                                          ComplexAnimator *parentAnimator) {
    Q_UNUSED(shift);
    if(!parentAnimator) return;
    for(const auto &key : anim_mKeys) {
        parentAnimator->ca_updateDescendatKeyFrame(key);
    }
}

void Gradient::addColorToList(const QColor &color) {
    auto newColorAnimator = enve::make_shared<ColorAnimator>();
    newColorAnimator->qra_setCurrentValue(color);
    addColorToList(newColorAnimator);
}

void Gradient::addColorToList(const qsptr<ColorAnimator>& newColorAnimator) {
    mColors << newColorAnimator;

    ca_addChild(newColorAnimator);
}

QColor Gradient::getColorAt(const int id) {
    if(id < 0 || id >= mColors.count()) return Qt::black;
    return mColors.at(id)->getColor();
}

ColorAnimator *Gradient::getColorAnimatorAt(const int id) {
    if(id < 0 || id >= mColors.count()) return nullptr;
    return mColors.at(id).get();
}

int Gradient::getColorCount() {
    return mColors.length();
}

QColor Gradient::getLastQGradientStopQColor() {
    if(mQGradientStops.isEmpty()) return Qt::black;
    return mQGradientStops.last().second;
}

QColor Gradient::getFirstQGradientStopQColor() {
    if(mQGradientStops.isEmpty()) return Qt::black;
    return mQGradientStops.first().second;
}

QGradientStops Gradient::getQGradientStops() {
    return mQGradientStops;
}

void Gradient::swapColors(const int id1, const int id2) {
    ca_swapChildAnimators(mColors.at(id1).get(),
                          mColors.at(id2).get());
    mColors.swap(id1, id2);
    updateQGradientStops(UpdateReason::userChange);
    prp_afterWholeInfluenceRangeChanged();
}

void Gradient::removeColor(const int id) {
    removeColor(mColors.at(id));
}

void Gradient::removeColor(const qsptr<ColorAnimator>& color) {
    ca_removeChild(color);
    mColors.removeOne(color);
    updateQGradientStops(UpdateReason::userChange);
    prp_afterWholeInfluenceRangeChanged();
}

void Gradient::addColor(const QColor &color) {
    addColorToList(color);
    updateQGradientStops(UpdateReason::userChange);
    prp_afterWholeInfluenceRangeChanged();
}

void Gradient::replaceColor(const int id, const QColor &color) {
    mColors.at(id)->qra_setCurrentValue(color);
    updateQGradientStops(UpdateReason::userChange);
    prp_afterWholeInfluenceRangeChanged();
}

void Gradient::startColorIdTransform(const int id) {
    if(mColors.count() <= id || id < 0) return;
    mColors.at(id)->prp_startTransform();
}

QGradientStops Gradient::getQGradientStopsAtAbsFrame(const qreal absFrame) {
    QGradientStops stops;
    const qreal inc = 1./(mColors.length() - 1);
    qreal cPos = 0.;
    for(int i = 0; i < mColors.length(); i++) {
        stops.append(QPair<qreal, QColor>(clamp(cPos, 0, 1),
                     mColors.at(i)->getColor(absFrame)) );
        cPos += inc;
    }
    return stops;
}

void Gradient::updateQGradientStops(const UpdateReason reason) {
    Q_UNUSED(reason);
    mQGradientStops.clear();
    const qreal inc = 1./(mColors.length() - 1);
    qreal cPos = 0;
    for(int i = 0; i < mColors.length(); i++) {
        mQGradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0, 1),
                                    mColors.at(i)->getColor()) );
        cPos += inc;
    }

    emit changed();
}

int Gradient::getReadWriteId() {
    return mReadWriteId;
}

void Gradient::clearReadWriteId() {
    mReadWriteId = -1;
}
