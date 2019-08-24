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

#ifndef DRAWPATH_H
#define DRAWPATH_H
#include <QPainterPath>
#include <QPainterPathStroker>
#include "skia/skiaincludes.h"

class DrawPath {
public:
    DrawPath();

    void startDrawingPath(const QPointF &startPos,
                          const QPointF &endPos);

    void startNewSubPath(const QPointF &startPos);

    void drawPathMove(const QPointF &pos);

    void finishDrawingPath(const bool close);

    void drawPath(SkCanvas *canvas);

    void updateFittedPath();

    void addNodeAt(const QPointF &pos);

    void incNodePosIdsForIdsAboveOrEqual(const int idT,
                                         const int inc = 1);

    void decNodePosIdsForIdsBelow(const int idT,
                                  const int inc = 1);

    void sortNodes();

    void removePosForNode(const int idT);

    struct DrawPathNode {
        DrawPathNode(const int idT,
                     const QPointF &posT) {
            posId = idT;
            pos = posT;
        }
        int posId;
        QPointF pos;
    };
protected:
    QPointF mStartPos;
    QPointF mEndPos;

    QList<DrawPathNode> mNodes;
    QList<QPointF> mOriginalPoses;
    SkPath mDrawPathOriginal;
    SkPath mDrawFitted;
    //QList<QPointF> mFittedPts;
    bool mClosed = false;
};

#endif // DRAWPATH_H
