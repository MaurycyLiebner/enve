#include "drawpath.h"
#include "pointhelpers.h"

DrawPath::DrawPath() {

}

void DrawPath::startDrawingPath(const QPointF &startPos,
                                const QPointF &endPos) {
    mOriginalPoses.clear();
    mNodes.clear();
    mDrawPathOriginal.reset();
    mDrawFitted.reset();
    startNewSubPath(startPos);
    mStartPos = startPos;
    mEndPos = endPos;
}

void DrawPath::startNewSubPath(const QPointF &startPos) {
    mOriginalPoses << startPos;
    mDrawPathOriginal.moveTo(startPos.x(), startPos.y());
}

void DrawPath::drawPathMove(const QPointF &pos) {
    mOriginalPoses << pos;
    mDrawPathOriginal.lineTo(pos.x(), pos.y());
}

void DrawPath::finishDrawingPath(const bool &close) {
    mClosed = close;
}

void DrawPath::drawPath(SkCanvas *canvas) {
    SkPaint paintT;
    paintT.setAntiAlias(true);
    paintT.setStyle(SkPaint::kStroke_Style);
    paintT.setColor(SK_ColorWHITE);
    paintT.setStrokeWidth(1.25);
    canvas->drawPath(mDrawPathOriginal, paintT);
    paintT.setColor(SK_ColorBLACK);
    paintT.setStrokeWidth(0.75);
    canvas->drawPath(mDrawPathOriginal, paintT);
    paintT.setColor(SK_ColorRED);
    canvas->drawPath(mDrawFitted, paintT);
}

#include "skia/skqtconversions.h"
void DrawPath::updateFittedPath() {
    QPointF p0;
    QPointF p3;
    mDrawFitted.reset();
    for(int nodeId = 0; nodeId < mNodes.count() - 1; nodeId++) {
        const DrawPathNode &thisNode = mNodes.at(nodeId);
        const DrawPathNode &nextNode = mNodes.at(nodeId + 1);

        p0 = thisNode.pos;
        if(nodeId == 0) {
            mDrawFitted.moveTo(toSkPoint(p0));
        }
        p3 = nextNode.pos;
        int minDataId = thisNode.posId;
        int maxDataId = nextNode.posId;
        QPointF p1 = p0;
        QPointF p2 = p3;
        gBezierLeastSquareV1V2({p0, p1, p2, p3}, mOriginalPoses,
                               minDataId, maxDataId);
        mDrawFitted.cubicTo(toSkPoint(p1),
                            toSkPoint(p2),
                            toSkPoint(p3));
    }
}

void DrawPath::addNodeAt(const QPointF &pos) {
    QPointF closestPos;
    qreal minDist = 1000000.;
    int closestPosId = 0;
    int idT = 0;
    QPointF lastPos;
    for(const QPointF &posT : mOriginalPoses) {
        if(idT > 0) {
            QPointF closestT =
                    gGetClosestPointOnLineSegment(lastPos, posT, pos);
            qreal distT = pointToLen(pos - closestT);
            if(distT < minDist) {
                minDist = distT;
                closestPos = closestT;
                closestPosId = idT;
            }
        }
        lastPos = posT;
        idT++;
    }
    incNodePosIdsForIdsAboveOrEqual(closestPosId);
    mOriginalPoses.insert(closestPosId, closestPos);
    mNodes << DrawPathNode(closestPosId, pos);
    sortNodes();
    updateFittedPath();
}

void DrawPath::incNodePosIdsForIdsAboveOrEqual(const int &idT, const int &inc) {
    for(int i = 0; i < mNodes.count(); i++) {
        const DrawPathNode &nodeT = mNodes.at(i);
        int id = nodeT.posId;
        if(id >= idT) mNodes.replace(i, DrawPathNode(id + inc, nodeT.pos));
    }
}

bool drawPathNodeSort(const DrawPath::DrawPathNode &n1,
                const DrawPath::DrawPathNode &n2) {
    return n1.posId < n2.posId;
}

void DrawPath::decNodePosIdsForIdsBelow(const int &idT, const int &inc) {
    for(int i = 0; i < mNodes.count(); i++) {
        const DrawPathNode &nodeT = mNodes.at(i);
        int id = nodeT.posId;
        if(id < idT) mNodes.replace(i, DrawPathNode(id - inc, nodeT.pos));
    }
}

void DrawPath::sortNodes() {
    std::sort(mNodes.begin(), mNodes.end(), drawPathNodeSort);
}

void DrawPath::removePosForNode(const int &idT) {
    mOriginalPoses.removeAt(idT);
}
