#include "drawpath.h"
#include "pointhelpers.h"

DrawPath::DrawPath() {

}

void DrawPath::startDrawingPath(const QPointF &startPos, const QPointF &endPos) {
    mOriginalPoses.clear();
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

void DrawPath::updateFittingPts() {

}

void DrawPath::addNodeAt(const QPointF &pos) {
    QPointF closestPos;
    qreal minDist = 1000000.;
    int closestPosId = 0;
    int idT = 0;
    QPointF lastPos;
    foreach(const QPointF &posT, mOriginalPoses) {
        if(idT > 0) {
            QPointF closestT =
                    getClosestPointOnLineSegment(lastPos, posT, pos);
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
    sortNodes();
}

void DrawPath::incNodePosIdsForIdsAboveOrEqual(const int &idT, const int &inc) {
    for(int i = 0; i < mNodePosIds.count(); i++) {
        int id = mNodePosIds.at(i);
        if(id >= idT) mNodePosIds.replace(i, id + inc);
    }
}

void DrawPath::decNodePosIdsForIdsBelow(const int &idT, const int &inc) {
    for(int i = 0; i < mNodePosIds.count(); i++) {
        int id = mNodePosIds.at(i);
        if(id < idT) mNodePosIds.replace(i, id - inc);
    }
}

void DrawPath::sortNodes() {
    qSort(mNodePosIds.begin(), mNodePosIds.end());
}

void DrawPath::removePosForNode(const int &idT) {
    mOriginalPoses.removeAt(idT);
}
