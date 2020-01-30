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

#include "sculptpathcollection.h"
#include "Animators/transformanimator.h"

SculptPathCollection::SculptPathCollection() :
    SculptPathCollectionBase("paths") {
//    connect(this, &ComplexAnimator::ca_childRemoved,
//            this, &SculptPathCollection::updatePathColors);
//    connect(this, &ComplexAnimator::ca_childAdded,
//            this, &SculptPathCollection::updatePathColors);
//    connect(this, &ComplexAnimator::ca_childMoved,
//            this, &SculptPathCollection::updatePathColors);
    connect(this, &SculptPathCollection::ca_childAdded,
            this, [this](Property* const child) {
        connect(child, &Property::prp_selectionChanged,
                this, &SculptPathCollection::updateVisibleChildren);
    });
    connect(this, &SculptPathCollection::ca_childRemoved,
            this, [this](Property* const child) {
        disconnect(child, &Property::prp_selectionChanged,
                   this, &SculptPathCollection::updateVisibleChildren);
    });
}

void SculptPathCollection::prp_writeProperty(eWriteStream &dst) const {
    SculptPathCollectionBase::prp_writeProperty(dst);
    dst.write(&mFillType, sizeof(SkPathFillType));
}

void SculptPathCollection::prp_readProperty(eReadStream &src) {
    SculptPathCollectionBase::prp_readProperty(src);
    src.read(&mFillType, sizeof(SkPathFillType));
}

SculptPathAnimator* SculptPathCollection::createNewSubPathAtRelPos(
        const QPointF &relPos, const qreal radius) {
    SkPath path;
    path.addCircle(relPos.x(), relPos.y(), radius);
    return createNewPath(path);
}

SculptPathAnimator *SculptPathCollection::createNewSubPathAtPos(
        const QPointF &absPos, const qreal radius) {
    const auto trans = getTransformAnimator();
    const auto relPos = trans ? trans->mapAbsPosToRel(absPos) : absPos;
    return createNewSubPathAtRelPos(relPos, radius);
}

QList<SculptPath> SculptPathCollection::getPathAtRelFrame(
        const qreal relFrame) const {
    QList<SculptPath> result;
    const auto& children = ca_getChildren();
    result.reserve(children.count());
    for(const auto& child : children) {
        result << SculptPath();
        const auto sculpt = static_cast<SculptPathAnimator*>(child.get());
        sculpt->deepCopyValue(relFrame, result.last());
    }
    return result;
}

void SculptPathCollection::applyTransform(const QMatrix &transform) const {
    const int iMax = ca_getNumberOfChildren() - 1;
    for(int i = 0; i <= iMax; i++) {
        const auto path = ca_getChildAt<SculptPathAnimator>(i);
        path->applyTransform(transform);
    }
}

void SculptPathCollection::setFillType(const SkPathFillType fillType) {
    if(mFillType == fillType) return;
    {
        UndoRedo ur;
        const auto oldValue = mFillType;
        const auto newValue = fillType;
        ur.fUndo = [this, oldValue]() {
            setFillType(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setFillType(newValue);
        };
        prp_addUndoRedo(ur);
    }
    mFillType = fillType;
    prp_afterWholeInfluenceRangeChanged();
    emit fillTypeChanged(fillType);
}

void SculptPathCollection::updateVisibleChildren() {
    bool allVisible = true;
    const auto& children = ca_getChildren();
    for(const auto& child : children) {
        if(child->prp_isSelected()) {
            allVisible = false;
            break;
        }
    }
    for(const auto& child : children) {
        const bool enabled = allVisible || child->prp_isSelected();
        child->prp_setDrawingOnCanvasEnabled(enabled);
    }
}

SculptPathAnimator *SculptPathCollection::createNewPath(const SkPath &path) {
    const auto newPath = createNewPath();
    newPath->getCurrentlyEdited()->setPath(path, 5);
    return newPath;
}

SculptPathAnimator *SculptPathCollection::createNewPath() {
    const auto newPath = enve::make_shared<SculptPathAnimator>();
    addChild(newPath);
    return newPath.get();
}

void SculptPathCollection::loadSkPath(const SkPath &path) {
    const QList<SkPath> paths = gBreakApart(path);
    for(const auto& sPath : paths) createNewPath(sPath);
}

void SculptPathCollection::sculpt(const SculptTarget target,
                                  const SculptMode mode,
                                  const SculptBrush &brush) {
    if(target == SculptTarget::position && mode == SculptMode::add) {
        createNewSubPathAtRelPos(brush.center(), brush.radius());
        return;
    }
    const int iMax = ca_getNumberOfChildren() - 1;
    for(int i = 0; i <= iMax; i++) {
        const auto path = ca_getChildAt<SculptPathAnimator>(i);
        if(!path->prp_drawsOnCanvas()) continue;
        path->sculpt(target, mode, brush);
    }
}
