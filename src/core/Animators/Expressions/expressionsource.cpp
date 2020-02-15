// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "expressionsource.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "simpletask.h"

ExpressionSource::ExpressionSource(QrealAnimator * const parent) :
    ExpressionSourceBase(parent),
    pathChanged([this]() {
        const auto src = source();
        updateSourcePath();
        const auto newSource = findSource();
        if(!newSource) setSourcePathValid(false);
        else if(newSource != src) setSource(newSource);
        else setSourcePathValid(true);
    }) {
    connect(parent, &QrealAnimator::prp_pathChanged,
            this, [this]() { pathChanged(); });
}

ExpressionValue::sptr ExpressionSource::sCreate(
        const QString &path, QrealAnimator * const parent) {
    const auto result = QSharedPointer<ExpressionSource>(
                new ExpressionSource(parent));
    result->setPath(path);
    return result;
}

qreal ExpressionSource::calculateValue(const qreal relFrame) const {
    const auto src = validSource();
    const auto prnt = parent();
    if(!src || !prnt) return 1;
    const auto absFrame = prnt->prp_relFrameToAbsFrameF(relFrame);
    const auto sourceRelFrame = src->prp_absFrameToRelFrame(absFrame);
    return src->getEffectiveValue(sourceRelFrame);
}

FrameRange ExpressionSource::identicalRange(const qreal relFrame) const {
    const auto src = validSource();
    const auto prnt = parent();
    if(!src || !prnt) return FrameRange::EMINMAX;
    const auto absFrame = prnt->prp_relFrameToAbsFrameF(relFrame);
    const auto sourceRelFrame = src->prp_absFrameToRelFrame(absFrame);
    const auto sourceAbsRange = src->prp_getIdenticalAbsRange(sourceRelFrame);
    const auto parentRelRange = prnt->prp_absRangeToRelRange(sourceAbsRange);
    return parentRelRange;
}

bool ExpressionSource::dependsOn(QrealAnimator * const source) const {
    const auto thisSource = this->source();
    if(!thisSource) return false;
    return source == thisSource ||
            thisSource->expressionDependsOn(source);
}

void ExpressionSource::setPath(const QString &path) {
    mPath = path;
    reloadSource();
    updateValue();
}

QrealAnimator *ExpressionSource::validSource() const {
    if(mSourcePathValid) return source();
    else return nullptr;
}

void ExpressionSource::setSourcePathValid(const bool valid) {
    if(mSourcePathValid == valid) return;
    mSourcePathValid = valid;
    updateValue();
    emit relRangeChanged(FrameRange::EMINMAX);
}

void ExpressionSource::setSource(QrealAnimator * const newSource) {
    auto& conn = ExpressionSourceBase::setSource(newSource);
    if(newSource) {
        setSourcePathValid(true);
        const auto prnt = parent();
        conn << connect(newSource, &Property::prp_absFrameRangeChanged,
                        this, [this, prnt](const FrameRange& absRange) {
            const auto relRange = prnt->prp_absRangeToRelRange(absRange);
            const bool currentFrameAffected = relRange.inRange(relFrame());
            if(currentFrameAffected) updateValue();
            emit relRangeChanged(relRange);
        });
        conn << connect(newSource, &QrealAnimator::prp_pathChanged,
                        this, [this]() { pathChanged(); });
    }
}

QrealAnimator *ExpressionSource::findSource() {
    const auto prnt = parent();
    if(!prnt) return nullptr;
    const auto searchCtxt = prnt->getParent();
    if(!searchCtxt) return nullptr;
    QrealAnimator* result = nullptr;
    const auto objs = mPath.split('.');
    const auto found = searchCtxt->ca_findPropertyWithPathRec(0, objs);
    if(found != prnt) result = enve::cast<QrealAnimator*>(found);
    if(result && (result == prnt ||
       result->expressionDependsOn(prnt))) {
        result = nullptr;
    }
    return result;
}

void ExpressionSource::reloadSource() {
    const auto newSource = findSource();
    setSource(newSource);
}

void ExpressionSource::updateSourcePath() {
    const auto src = source();
    const auto prnt = parent();
    if(!src || !prnt) return;
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
