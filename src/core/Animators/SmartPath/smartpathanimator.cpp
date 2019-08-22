#include "smartpathanimator.h"
#include "Animators/qrealpoint.h"
#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"

SmartPathAnimator::SmartPathAnimator() :
    GraphAnimator("path"),
    startPathChange([this]() { startPathChangeExec(); }),
    pathChanged([this]() { pathChangedExec(); }),
    cancelPathChange([this]() { cancelPathChangeExec(); }),
    finishPathChange([this]() { finishPathChangeExec(); }) {
    setPointsHandler(enve::make_shared<PathPointsHandler>(this));
}

SmartPathAnimator::SmartPathAnimator(const SkPath &path) :
    SmartPathAnimator() {
    mBaseValue.setPath(path);
    mCurrentPath = path;
    updateAllPoints();
}

SmartPathAnimator::SmartPathAnimator(const SmartPath &baseValue) :
    SmartPathAnimator() {
    mBaseValue = baseValue;
    mPathUpToDate = false;
    updateAllPoints();
}

#include "typemenu.h"
#include "document.h"
void SmartPathAnimator::setupTreeViewMenu(PropertyMenu * const menu) {
    const auto spClipboard = Document::sInstance->getSmartPathClipboard();
//    if(spClipboard) {
        menu->addPlainAction("Paste Path", [this, spClipboard]() {
            pastePath(spClipboard->path());
        })->setEnabled(spClipboard);
//    }
    menu->addPlainAction("Copy Path", [this] {
        const auto spClipboard = enve::make_shared<SmartPathClipboard>(mBaseValue);
        Document::sInstance->replaceClipboard(spClipboard);
    });
    menu->addSeparator();
    Animator::setupTreeViewMenu(menu);
}

void SmartPathAnimator::readProperty(eReadStream& src) {
    readKeys(src);
    src >> mBaseValue;
    prp_afterWholeInfluenceRangeChanged();
    updateAllPoints();
}

void SmartPathAnimator::graph_getValueConstraints(
        GraphKey *key, const QrealPointType &type,
        qreal &minValue, qreal &maxValue) const {
    if(type == QrealPointType::KEY_POINT) {
        minValue = key->getRelFrame();
        maxValue = minValue;
        //getFrameConstraints(key, type, minValue, maxValue);
    } else {
        minValue = -DBL_MAX;
        maxValue = DBL_MAX;
    }
}

void SmartPathAnimator::actionDisconnectNodes(const int node1Id,
                                              const int node2Id) {
    for(const auto &key : anim_mKeys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        auto& keyPath = spKey->getValue();
        keyPath.actionDisconnectNodes(node1Id, node2Id);
    }
    mBaseValue.actionDisconnectNodes(node1Id, node2Id);
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::updateAllPoints() {
    const auto handler = getPointsHandler();
    const auto pathHandler = static_cast<PathPointsHandler*>(handler);
    pathHandler->updateAllPoints();
}
