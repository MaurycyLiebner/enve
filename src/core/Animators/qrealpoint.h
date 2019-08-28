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

#ifndef QREALPOINT_H
#define QREALPOINT_H
#include <QPainter>
#include "../smartPointers/ememory.h"

class GraphKey;

enum QrealPointType : short {
    START_POINT,
    END_POINT,
    KEY_POINT
};

class QrealPoint : public StdSelfRef {
    e_OBJECT
public:
    QrealPoint(QrealPointType type,
               GraphKey * const parentKey,
               const qreal radius = 10);

    qreal getRelFrame();
    qreal getAbsFrame();

    QPointF getSavedFrameAndValue() const;
    qreal getSavedFrame() const;
    qreal getSavedValue() const;

    void setFrameAndValue(const qreal relFrame,
                          const qreal value);

    void setRelFrame(const qreal frame);
    void setAbsFrame(const qreal absFrame);

    qreal getValue();

    void setValue(const qreal value);

    bool isSelected();

    bool isNear(const qreal absFrame, const qreal value,
                const qreal pixelsPerFrame, const qreal pixelsPerValue);

    void moveBy(const qreal dFrame, const qreal dValue);
    void moveTo(const qreal frameT, const qreal value);

    void draw(QPainter * const p, const QColor &paintColor);

    void setSelected(const bool selected);

    bool isKeyPt() const
    { return mType == KEY_POINT; }
    bool isStartPt() const
    { return mType == START_POINT; }
    bool isEndPt() const
    { return mType == END_POINT; }
    bool isCtrlPt() const
    { return isEndPt() || isStartPt(); }

    bool isEnabled();

    GraphKey *getParentKey()
    { return mParentKey; }

    void startTransform();
    void finishTransform();

    QrealPointType getType() const
    { return mType; }

    void setHovered(const bool hovered)
    { mHovered = hovered; }
private:
    bool mIsSelected = false;
    bool mHovered = false;
    QrealPointType mType;
    GraphKey *mParentKey;
    qreal mRadius;
};


#endif // QREALPOINT_H
