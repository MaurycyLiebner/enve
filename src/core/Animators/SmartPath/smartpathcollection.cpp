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

#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"
#include "Animators/transformanimator.h"

SmartPathCollection::SmartPathCollection() :
    SmartPathCollectionBase("paths") {}

void SmartPathCollection::prp_writeProperty(eWriteStream &dst) const {
    SmartPathCollectionBase::prp_writeProperty(dst);
    dst.write(&mFillType, sizeof(SkPath::FillType));
}

void SmartPathCollection::prp_readProperty(eReadStream &src) {
    SmartPathCollectionBase::prp_readProperty(src);
    src.read(&mFillType, sizeof(SkPath::FillType));
}

SmartNodePoint *SmartPathCollection::createNewSubPathAtRelPos(const QPointF &relPos) {
    const auto newPath = createNewPath();
    const auto handler = newPath->getPointsHandler();
    const auto pathHandler = static_cast<PathPointsHandler*>(handler);
    return pathHandler->addNewAtEnd(relPos);
}

SmartNodePoint *SmartPathCollection::createNewSubPathAtPos(const QPointF &absPos) {
    const auto newPath = createNewPath();
    const auto handler = newPath->getPointsHandler();
    const auto pathHandler = static_cast<PathPointsHandler*>(handler);
    const auto trans = handler->transform();
    const auto relPos = trans ? trans->mapAbsPosToRel(absPos) : absPos;
    return pathHandler->addNewAtEnd(relPos);
}

SkPath SmartPathCollection::getPathAtRelFrame(const qreal relFrame) const {
    SkPath result;
    for(const auto& child : ca_mChildAnimators) {
        const auto path = static_cast<SmartPathAnimator*>(child.get());
        const auto mode = path->getMode();
        if(mode == SmartPathAnimator::NORMAL)
            result.addPath(path->getPathAtRelFrame(relFrame));
        else {
            SkPathOp op{SkPathOp::kUnion_SkPathOp};
            switch(mode) {
                case(SmartPathAnimator::NORMAL):
                case(SmartPathAnimator::ADD):
                    op = SkPathOp::kUnion_SkPathOp;
                    break;
                case(SmartPathAnimator::REMOVE):
                    op = SkPathOp::kDifference_SkPathOp;
                    break;
                case(SmartPathAnimator::REMOVE_REVERSE):
                    op = SkPathOp::kReverseDifference_SkPathOp;
                    break;
                case(SmartPathAnimator::INTERSECT):
                    op = SkPathOp::kIntersect_SkPathOp;
                    break;
                case(SmartPathAnimator::EXCLUDE):
                    op = SkPathOp::kXOR_SkPathOp;
                    break;
                case(SmartPathAnimator::DIVIDE):
                    const SkPath skPath = path->getPathAtRelFrame(relFrame);
                    SkPath intersect;
                    op = SkPathOp::kIntersect_SkPathOp;
                    if(!Op(result, skPath, op, &intersect))
                        RuntimeThrow("Operation Failed");
                    op = SkPathOp::kDifference_SkPathOp;
                    if(!Op(result, skPath, op, &result))
                        RuntimeThrow("Operation Failed");
                    result.addPath(intersect);
                    continue;
            }
            if(!Op(result, path->getPathAtRelFrame(relFrame), op, &result))
                RuntimeThrow("Operation Failed");
        }
    }
    result.setFillType(mFillType);
    return result;
}

void SmartPathCollection::applyTransform(const QMatrix &transform) const {
    const int iMax = ca_getNumberOfChildren() - 1;
    for(int i = 0; i <= iMax; i++) {
        const auto path = ca_getChildAt<SmartPathAnimator>(i);
        path->applyTransform(transform);
    }
}

void SmartPathCollection::loadSkPath(const SkPath &path) {
    const QList<SkPath> paths = gBreakApart(path);
    for(const auto& sPath : paths) createNewPath(sPath);
}
