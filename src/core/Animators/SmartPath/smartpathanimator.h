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

#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "../interpolationanimatort.h"
#include "differsinterpolate.h"
#include "../../ReadWrite/basicreadwrite.h"
#include "smartpathkey.h"
#include "../../MovablePoints/segment.h"
#include "../../skia/skiahelpers.h"
#include "simpletask.h"

class SmartPathAnimator : public GraphAnimator {
    e_OBJECT
    Q_OBJECT
protected:
    SmartPathAnimator();
    SmartPathAnimator(const SkPath& path);
    SmartPathAnimator(const SmartPath& baseValue);
public:
    enum class Mode {
        normal,
        add, remove, removeReverse,
        intersect, exclude,
        divide
    };

    bool SWT_isSmartPathAnimator() const { return true; }

    QMimeData *SWT_createMimeData();

    void prp_setupTreeViewMenu(PropertyMenu * const menu);

    void prp_drawCanvasControls(
            SkCanvas * const canvas, const CanvasMode mode,
            const float invScale, const bool ctrlPressed);

    void prp_afterChangedAbsRange(const FrameRange &range,
                                  const bool clip = true);

    void prp_readProperty(eReadStream& src);
    void prp_writeProperty(eWriteStream& dst) const;

    void anim_setAbsFrame(const int frame);
    void anim_addKeyAtRelFrame(const int relFrame);
    stdsptr<Key> anim_createKey();
    void anim_afterKeyOnCurrentFrameChanged(Key* const key);

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType type,
            qreal &minValue, qreal &maxValue) const;

    void deepCopySmartPathFromRelFrame(const int relFrame,
                                       SmartPath &result) const;

    SkPath getPathAtAbsFrame(const qreal frame)
    { return getPathAtRelFrame(prp_absFrameToRelFrameF(frame)); }
    SkPath getPathAtRelFrame(const qreal frame);

    SmartPath * getCurrentlyEditedPath() const
    { return mPathBeingChanged_d; }

    bool isClosed() const
    { return mBaseValue.isClosed(); }

    void beforeBinaryPathChange();
    void afterBinaryPathChange();

    void prp_startTransform();
    SimpleTaskScheduler pathChanged;
    void pathChangedExec();
    void prp_cancelTransform();
    void prp_finishTransform();

    void actionRemoveNode(const int nodeId, const bool approx);
    int actionAddNewAtStart(const QPointF &relPos);
    int actionAddNewAtStart(const NormalNodeData &data);
    int actionAddNewAtEnd(const QPointF &relPos);
    int actionAddNewAtEnd(const NormalNodeData &data);
    int actionInsertNodeBetween(const int node1Id, const int node2Id,
                                const qreal t);
    void actionConnectNodes(const int node1Id, const int node2Id);
    void actionMergeNodes(const int node1Id, const int node2Id);
    void actionMoveNodeBetween(const int nodeId, const int prevNodeId,
                               const int nextNodeId);
    void actionClose();
    void actionDisconnectNodes(const int node1Id, const int node2Id);
    void actionReverseCurrent();
    void actionReverseAll();

    void actionAppendMoveAllFrom(SmartPathAnimator * const other);

    void actionPrependMoveAllFrom(SmartPathAnimator * const other);

    bool hasDetached() const
    { return mBaseValue.hasDetached(); }

    qsptr<SmartPathAnimator> createFromDetached();

    void applyTransform(const QMatrix &transform);

    const SkPath& getCurrentPath();

    void setMode(const Mode mode);
    Mode getMode() const { return mMode; }

    void pastePath(const SmartPath& path)
    { return pastePath(anim_getCurrentRelFrame(), path); }
    void pastePath(const int frame, SmartPath path);

    void setPathColor(const QColor& color)
    { mPathColor = color; }
    const QColor& getPathColor() const
    { return mPathColor; }
signals:
    void pathBlendModeChagned(Mode);
    void emptied();
protected:
    SmartPath& getBaseValue()
    { return mBaseValue; }
private:
    int actionAddFirstNode(const QPointF &relPos);
    int actionAddFirstNode(const NormalNodeData &data);

    void updateBaseValue();

    void updateAllPoints();

    void deepCopySmartPathFromRelFrame(const int relFrame,
                                       SmartPathKey * const prevKey,
                                       SmartPathKey * const nextKey,
                                       SmartPathKey * const keyAtFrame,
                                       SmartPath &result) const;

    bool mPathChanged = false;
    bool mPathUpToDate = true;
    SkPath mCurrentPath;
    SmartPath mBaseValue;
    SmartPath * mPathBeingChanged_d = &mBaseValue;
    Mode mMode = Mode::normal;
    QColor mPathColor = Qt::white;
};

#endif // SMARTPATHANIMATOR_H
