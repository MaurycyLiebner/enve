#ifndef GRAPHANIMATOR_H
#define GRAPHANIMATOR_H
#include "animator.h"
#define GetAsGK(key) GetAsPtr(key, GraphKey)
class GraphKey;
enum QrealPointType : short;
class GraphAnimator : public Animator {
public:
    GraphAnimator(const QString& name);

    void anim_appendKey(const stdsptr<Key> &newKey);
    void anim_removeKey(const stdsptr<Key> &keyToRemove);

    virtual void graph_getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minMoveValue, qreal &maxMoveValue) const = 0;
    virtual ValueRange graph_getMinAndMaxValues() const;
    virtual ValueRange graph_getMinAndMaxValuesBetweenFrames(
            const int &startFrame, const int &endFrame) const;
    virtual qreal graph_clampGraphValue(const qreal &value) { return value; }
    virtual void graph_updateKeysPath();
    virtual void graph_constrainCtrlsFrameValues();
    bool SWT_isGraphAnimator() const { return true; }

    void graph_addKeysInRectToList(const QRectF &frameValueRect,
                                   QList<GraphKey*> &keys);

    void graph_incSelectedForGraph() {
        if(!graph_mSelected) graph_updateKeysPath();
        graph_mSelected++;
    }

    void graph_decSelectedForGraph() {
        graph_mSelected--;
    }

    bool graph_isSelectedForGraph() {
        return graph_mSelected;
    }

    QColor graph_getAnimatorColor(void *ptr) const {
        for(const auto& x : graph_mAnimatorColors) {
            if(x.first == ptr) {
                return x.second;
            }
        }
        return QColor();
    }

    void graph_setAnimatorColor(void *ptr, const QColor &color) {
        graph_mAnimatorColors[ptr] = color;
    }

    void graph_removeAnimatorColor(void *ptr) {
        graph_mAnimatorColors.erase(ptr);
    }

    bool graph_isCurrentAnimator(void *ptr) const {
        return graph_mAnimatorColors.find(ptr) != graph_mAnimatorColors.end();
    }

    void graph_drawKeysPath(QPainter * const p,
                            const QColor &paintColor) const;

    void graph_getFrameValueConstraints(GraphKey *key,
                                  const QrealPointType& type,
                                  qreal &minMoveFrame,
                                  qreal &maxMoveFrame,
                                  qreal &minMoveValue,
                                  qreal &maxMoveValue) const;

    QrealPoint *graph_getPointAt(const qreal &value,
                                 const qreal &frame,
                                 const qreal &pixelsPerFrame,
                                 const qreal &pixelsPerValUnit);
    void graph_changeSelectedKeysFrameAndValueStart(const QPointF &frameVal);
    void graph_changeSelectedKeysFrameAndValue(const QPointF& frameVal);
    void graph_enableCtrlPtsForSelected();
    void graph_setCtrlsModeForSelectedKeys(const CtrlsMode &mode);
    void graph_getSelectedSegments(QList<QList<GraphKey*>> &segments);
protected:
    QPainterPath graph_mKeysPath;
private:
    void graph_getFrameConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minMoveFrame, qreal &maxMoveFrame) const;

    int graph_mSelected = 0;

    std::map<void*, QColor> graph_mAnimatorColors;
};

#endif // GRAPHANIMATOR_H
