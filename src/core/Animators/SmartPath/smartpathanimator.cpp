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

#include "smartpathanimator.h"
#include "Animators/qrealpoint.h"
#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"

SmartPathAnimator::SmartPathAnimator() :
    GraphAnimator("path"),
    pathChanged([this]() { pathChangedExec(); }) {
    const auto ptsHandler = enve::make_shared<PathPointsHandler>(this);
    QObject::connect(this, &Property::prp_currentFrameChanged,
                     this, [ptsHandler] {
        ptsHandler->updateAllPoints();
    });
    setPointsHandler(ptsHandler);
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

QMimeData *SmartPathAnimator::SWT_createMimeData() {
    return new eMimeData(QList<SmartPathAnimator*>() << this);
}

#include "typemenu.h"
#include "Private/document.h"
#include <QInputDialog>
void SmartPathAnimator::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    const auto parentWidget = menu->getParentWidget();
    menu->addPlainAction("Rename", [this, parentWidget]() {
        bool ok;
        const QString text = QInputDialog::getText(parentWidget, tr("New name dialog"),
                                                   tr("Name:"), QLineEdit::Normal,
                                                   prp_getName(), &ok);
        if(ok) prp_setName(text);
    });
    const auto spClipboard = Document::sInstance->getSmartPathClipboard();
    menu->addPlainAction("Paste Path", [this, spClipboard]() {
        pastePath(spClipboard->path());
    })->setEnabled(spClipboard);
    menu->addPlainAction("Copy Path", [this] {
        const auto spClipboard = enve::make_shared<SmartPathClipboard>(mBaseValue);
        Document::sInstance->replaceClipboard(spClipboard);
    });
    menu->addSeparator();
    const PropertyMenu::PlainSelectedOp<SmartPathAnimator> dOp =
    [](SmartPathAnimator* const eff) {
        const auto parent = eff->getParent<DynamicComplexAnimatorBase<SmartPathAnimator>>();
        parent->removeChild(eff->ref<SmartPathAnimator>());
    };
    menu->addPlainAction("Delete Path(s)", dOp);
    menu->addSeparator();
    Animator::prp_setupTreeViewMenu(menu);
}

void SmartPathAnimator::prp_drawCanvasControls(
        SkCanvas * const canvas, const CanvasMode mode,
        const float invScale, const bool ctrlPressed) {
    SkiaHelpers::drawOutlineOverlay(canvas, mCurrentPath, invScale,
                                    toSkMatrix(getTransform()),
                                    toSkColor(mPathColor));
    Property::prp_drawCanvasControls(canvas, mode, invScale, ctrlPressed);
}

void SmartPathAnimator::prp_afterChangedAbsRange(
        const FrameRange &range, const bool clip) {
    if(range.inRange(anim_getCurrentAbsFrame())) updateBaseValue();
    GraphAnimator::prp_afterChangedAbsRange(range, clip);
}

void SmartPathAnimator::prp_readProperty(eReadStream& src) {
    anim_readKeys(src);
    src >> mBaseValue;
    src.read(&mMode, sizeof(Mode));
    if(src.evFileVersion() > 3) {
        QString name; src >> name;
        prp_setName(name);
    }
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::prp_writeProperty(eWriteStream &dst) const {
    anim_writeKeys(dst);
    dst << mBaseValue;
    dst.write(&mMode, sizeof(Mode));
    dst << prp_getName();
}

void SmartPathAnimator::anim_setAbsFrame(const int frame) {
    if(frame == anim_getCurrentAbsFrame()) return;
    const int lastRelFrame = anim_getCurrentRelFrame();
    Animator::anim_setAbsFrame(frame);
    const bool diff = prp_differencesBetweenRelFrames(
                anim_getCurrentRelFrame(), lastRelFrame);
    if(diff) {
        updateBaseValue();
        prp_afterChangedCurrent(UpdateReason::frameChange);
    }
}

void SmartPathAnimator::anim_addKeyAtRelFrame(const int relFrame) {
    if(anim_getKeyAtRelFrame(relFrame)) return;
    const auto newKey = enve::make_shared<SmartPathKey>(this);
    newKey->setRelFrame(relFrame);
    deepCopySmartPathFromRelFrame(relFrame, newKey->getValue());
    anim_appendKey(newKey);
}

stdsptr<Key> SmartPathAnimator::anim_createKey() {
    return enve::make_shared<SmartPathKey>(this);
}

void SmartPathAnimator::anim_afterKeyOnCurrentFrameChanged(Key * const key) {
    const auto spk = static_cast<SmartPathKey*>(key);
    if(spk) mPathBeingChanged_d = &spk->getValue();
    else mPathBeingChanged_d = &mBaseValue;
}

void SmartPathAnimator::graph_getValueConstraints(
        GraphKey *key, const QrealPointType type,
        qreal &minValue, qreal &maxValue) const {
    if(type == QrealPointType::keyPt) {
        minValue = key->getRelFrame();
        maxValue = minValue;
        //getFrameConstraints(key, type, minValue, maxValue);
    } else {
        minValue = -DBL_MAX;
        maxValue = DBL_MAX;
    }
}

void SmartPathAnimator::deepCopySmartPathFromRelFrame(
        const int relFrame, SmartPath &result) const {
    const auto prevKey = anim_getPrevKey<SmartPathKey>(relFrame);
    const auto nextKey = anim_getNextKey<SmartPathKey>(relFrame);
    const auto keyAtFrame = anim_getKeyAtRelFrame<SmartPathKey>(relFrame);
    deepCopySmartPathFromRelFrame(relFrame, prevKey, nextKey,
                                  keyAtFrame, result);
}

SkPath SmartPathAnimator::getPathAtRelFrame(const qreal frame) {
    const auto diff = prp_differencesBetweenRelFrames(
                qRound(frame), anim_getCurrentRelFrame());
    if(!diff) return getCurrentPath();
    const auto pn = anim_getPrevAndNextKeyIdF(frame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    const auto prevKey = anim_getKeyAtIndex<SmartPathKey>(prevId);
    const auto nextKey = anim_getKeyAtIndex<SmartPathKey>(nextId);
    const bool adjKeys = pn.second - pn.first == 1;
    const auto keyAtRelFrame = adjKeys ? nullptr :
           anim_getKeyAtIndex<SmartPathKey>(pn.first + 1);
    if(keyAtRelFrame) return keyAtRelFrame->getValue().getPathAt();
    if(prevKey && nextKey) {
        SkPath result;
        const qreal nWeight = graph_prevKeyWeight(prevKey, nextKey, frame);
        SmartPath sPath;
        const auto& prevPath = prevKey->getValue();
        const auto& nextPath = nextKey->getValue();
        gInterpolate(prevPath, nextPath, nWeight, sPath);
        return sPath.getPathAt();
    } else if(!prevKey && nextKey) {
        return nextKey->getPath();
    } else if(prevKey && !nextKey) {
        return prevKey->getPath();
    }
    return mBaseValue.getPathAt();
}

void SmartPathAnimator::beforeBinaryPathChange() {
    prp_startTransform();
}

void SmartPathAnimator::afterBinaryPathChange() {
    pathChangedExec();
    prp_finishTransform();
}

void SmartPathAnimator::prp_startTransform() {
    if(mPathChanged) return;
    mPathChanged = true;
    if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
        anim_saveCurrentValueAsKey();
    }
    if(const auto key = anim_getKeyOnCurrentFrame<SmartPathKey>()) {
        key->startValueTransform();
    } else {
        mBaseValue.save();
    }
}

void SmartPathAnimator::pathChangedExec() {
    const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
    if(spk) anim_updateAfterChangedKey(spk);
    else prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::prp_cancelTransform() {
    if(!mPathChanged) return;
    mPathChanged = false;
    mPathBeingChanged_d->restore();
    if(const auto key = anim_getKeyOnCurrentFrame<SmartPathKey>()) {
        key->cancelValueTransform();
        anim_updateAfterChangedKey(key);
    } else {
        mBaseValue.restore();
        prp_afterWholeInfluenceRangeChanged();
    }
}

void SmartPathAnimator::prp_finishTransform() {
    if(!mPathChanged) return;
    mPathChanged = false;
    if(const auto key = anim_getKeyOnCurrentFrame<SmartPathKey>()) {
        key->finishValueTransform();
    } else {
        const auto oldValue = mBaseValue.getSaved();
        const auto newValue = mBaseValue.getNodesRef();
        UndoRedo ur;
        ur.fUndo = [this, oldValue]() {
            mBaseValue = oldValue;
            prp_afterWholeInfluenceRangeChanged();
        };
        ur.fRedo = [this, newValue]() {
            mBaseValue = newValue;
            prp_afterWholeInfluenceRangeChanged();
        };
        prp_addUndoRedo(ur);
    }
}

qsptr<SmartPathAnimator> SmartPathAnimator::createFromDetached() {
    if(!hasDetached()) return nullptr;
    const auto baseDetached = mBaseValue.getAndClearLastDetached();
    SmartPath baseSmartPath(baseDetached);
    const auto newAnim = enve::make_shared<SmartPathAnimator>(baseSmartPath);
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        auto& sp = spKey->getValue();
        const auto keyDetached = sp.getAndClearLastDetached();
        SmartPath keySmartPath(keyDetached);
        const auto newKey = enve::make_shared<SmartPathKey>(
                    keySmartPath, key->getRelFrame(), newAnim.get());
        newAnim->anim_appendKey(newKey);
    }
    return newAnim;
}

void SmartPathAnimator::applyTransform(const QMatrix &transform) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().applyTransform(transform);
    }
    mBaseValue.applyTransform(transform);
    prp_afterWholeInfluenceRangeChanged();
}

const SkPath &SmartPathAnimator::getCurrentPath() {
    if(!mPathUpToDate) {
        mCurrentPath = getCurrentlyEditedPath()->getPathAt();
        mPathUpToDate = true;
    }
    return mCurrentPath;
}

void SmartPathAnimator::setMode(const SmartPathAnimator::Mode mode) {
    if(mMode == mode) return;
    mMode = mode;
    prp_afterWholeInfluenceRangeChanged();
    emit pathBlendModeChagned(mode);
}

void SmartPathAnimator::pastePath(const int frame, SmartPath path) {
    if(!anim_isRecording()) {
        mBaseValue = path;
        return prp_afterWholeInfluenceRangeChanged();
    }
    const bool pasteClosed = path.isClosed();
    const bool baseClosed = mBaseValue.isClosed();

    if(pasteClosed != baseClosed) {
        if(pasteClosed) path.actionOpen();
        else {
            const auto& keys = anim_getKeys();
            for(const auto &key : keys) {
                const auto spKey = static_cast<SmartPathKey*>(key);
                auto& sp = spKey->getValue();
                sp.actionOpen();
            }
        }
    }

    const int pasteNodes = path.getNodeCount();
    const int baseNodes = mBaseValue.getNodeCount();
    const int addNodes = pasteNodes - baseNodes;
    if(addNodes > 0) {
        const auto& keys = anim_getKeys();
        for(const auto &key : keys) {
            const auto spKey = static_cast<SmartPathKey*>(key);
            auto& sp = spKey->getValue();
            sp.addDissolvedNodes(addNodes);
        }
    } else if(addNodes < 0) {
        path.addDissolvedNodes(-addNodes);
    }

    auto key = anim_getKeyAtRelFrame<SmartPathKey>(frame);
    if(key) key->setValue(path);
    else {
        const auto newKey = enve::make_shared<SmartPathKey>(path, frame, this);
        anim_appendKey(newKey);
    }
}

void SmartPathAnimator::updateBaseValue() {
    const auto prevK = anim_getPrevKey<SmartPathKey>(anim_getCurrentRelFrame());
    const auto nextK = anim_getNextKey<SmartPathKey>(anim_getCurrentRelFrame());
    const auto keyAtFrame = anim_getKeyOnCurrentFrame<SmartPathKey>();
    mPathUpToDate = false;
    deepCopySmartPathFromRelFrame(anim_getCurrentRelFrame(),
                                  prevK, nextK, keyAtFrame,
                                  mBaseValue);
}

void SmartPathAnimator::updateAllPoints() {
    const auto handler = getPointsHandler();
    const auto pathHandler = static_cast<PathPointsHandler*>(handler);
    pathHandler->updateAllPoints();
}

void SmartPathAnimator::deepCopySmartPathFromRelFrame(
        const int relFrame,
        SmartPathKey * const prevKey,
        SmartPathKey * const nextKey,
        SmartPathKey * const keyAtFrame,
        SmartPath &result) const {
    if(keyAtFrame) {
        result = keyAtFrame->getValue();
    } else if(prevKey && nextKey) {
        const qreal nWeight = graph_prevKeyWeight(prevKey, nextKey, relFrame);
        const auto& prevPath = prevKey->getValue();
        const auto& nextPath = nextKey->getValue();
        gInterpolate(prevPath, nextPath, nWeight, result);
    } else if(prevKey) {
        result = prevKey->getValue();
    } else if(nextKey) {
        result = nextKey->getValue();
    } else {
        if(&result == &mBaseValue) return;
        result = mBaseValue;
    }
}
