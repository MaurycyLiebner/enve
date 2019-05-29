#include "smartvectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "pointhelpers.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Animators/gradientpoints.h"
#include "Animators/effectanimators.h"
#include "Animators/transformanimator.h"
#include "MovablePoints/segment.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "PathEffects/patheffectanimators.h"

SmartVectorPath::SmartVectorPath() :
    PathBox(BoundingBoxType::TYPE_VECTOR_PATH) {
    setName("Path");
    mPathAnimator = SPtrCreate(SmartPathCollection)();
    const auto updater = SPtrCreate(NodePointUpdater)(this);
    mPathAnimator->prp_setOwnUpdater(updater);
    ca_prependChildAnimator(mPathEffectsAnimators.data(),
                            mPathAnimator);
}

bool SmartVectorPath::differenceInEditPathBetweenFrames(
        const int &frame1, const int &frame2) const {
    return mPathAnimator->prp_differencesBetweenRelFrames(frame1, frame2);
}

void SmartVectorPath::loadSkPath(const SkPath &path) {
    mPathAnimator->loadSkPath(path);
}

SmartPathCollection *SmartVectorPath::getPathAnimator() {
    return mPathAnimator.data();
}

#include "typemenu.h"
void SmartVectorPath::addActionsToMenu(BoxTypeMenu * const menu) {
    PathBox::addActionsToMenu(menu);
    BoxTypeMenu::PlainOp<SmartVectorPath> op = [](SmartVectorPath * box) {
        box->applyCurrentTransformation();
    };
    menu->addPlainAction("Apply Transformation", op);
}

void SmartVectorPath::applyCurrentTransformation() {
    mNReasonsNotToApplyUglyTransform++;
//    mPathAnimator->applyTransformToPoints(
//                mTransformAnimator->getCurrentTransformationMatrix());

    mTransformAnimator->reset();
    centerPivotPosition();
    mNReasonsNotToApplyUglyTransform--;
}

SkPath SmartVectorPath::getPathAtRelFrameF(const qreal &relFrame) {
     return mPathAnimator->getPathAtRelFrame(relFrame);
}

void SmartVectorPath::getMotionBlurProperties(QList<Property*> &list) const {
    PathBox::getMotionBlurProperties(list);
    list.append(mPathAnimator.get());
}

QList<qsptr<SmartVectorPath>> SmartVectorPath::breakPathsApart_k() {
    QList<qsptr<SmartVectorPath>> result;
    const int iMax = mPathAnimator->ca_getNumberOfChildren() - 1;
    if(iMax < 1) return result;
    for(int i = iMax; i >= 0; i--) {
        const auto srcPath = mPathAnimator->ca_takeChildAt<SmartPathAnimator>(i);
        const auto newPath = SPtrCreate(SmartVectorPath)();
        copyPathBoxDataTo(newPath.get());
        newPath->getPathAnimator()->addPath(srcPath);
        result.append(newPath);
    }
    removeFromParent_k();
    return result;
}
