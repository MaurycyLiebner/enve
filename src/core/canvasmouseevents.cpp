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

#include "canvas.h"
#include "Boxes/paintbox.h"
#include "Boxes/sculptpathbox.h"
#include "Animators/SculptPath/sculptbrush.h"
#include "Boxes/textbox.h"
#include "Boxes/rectangle.h"
#include "Boxes/circle.h"
#include "Private/document.h"
#include "MovablePoints/pathpivot.h"

SculptPathBox* Canvas::newSculptPathBox(const QPointF &pos) {
    const auto sculptBox = enve::make_shared<SculptPathBox>();
    sculptBox->setStrokeBrush(mDocument.fBrush);
    sculptBox->planCenterPivotPosition();
    mCurrentContainer->addContained(sculptBox);
    sculptBox->setAbsolutePos(pos);
    clearBoxesSelection();
    clearPointsSelection();
    addBoxToSelection(sculptBox.get());
    return sculptBox.get();
}

void Canvas::newPaintBox(const QPointF &pos) {
    const auto paintBox = enve::make_shared<PaintBox>();
    paintBox->planCenterPivotPosition();
    mCurrentContainer->addContained(paintBox);
    paintBox->setAbsolutePos(pos);
    clearBoxesSelection();
    clearPointsSelection();
    addBoxToSelection(paintBox.get());
}

void Canvas::mousePressEvent(const MouseEvent &e) {
    if(mStylusDrawing) return;
    if(isPreviewingOrRendering()) return;
    if(e.fMouseGrabbing && e.fButton == Qt::LeftButton) return;
    if(mCurrentMode == CanvasMode::paint) {
        if(e.fButton == Qt::LeftButton) {
            if(!mPaintTarget.isValid()) newPaintBox(e.fPos);
            mPaintTarget.paintPress(e.fPos, e.fTimestamp, 0.5,
                                    0, 0, mDocument.fBrush);
        }
    } else if(mCurrentMode == CanvasMode::sculptPath) {
        if(e.fButton == Qt::LeftButton) {
            sculptPress(e.fPos, 1);
            e.fGrabMouse();
        } else if(e.fButton == Qt::RightButton) {
            sculptCancel();
        }
    } else {
        if(e.fButton == Qt::LeftButton) {
            handleLeftButtonMousePress(e);
        } else if(e.fButton == Qt::RightButton) {
            handleRightButtonMousePress(e);
        }
    }
}

void Canvas::mouseMoveEvent(const MouseEvent &e) {
    if(mStylusDrawing) return;
    if(isPreviewingOrRendering()) return;

    const bool leftPressed = e.fButtons & Qt::LeftButton;

    if(!leftPressed && !e.fMouseGrabbing) {
        const auto lastHoveredBox = mHoveredBox;
        const auto lastHoveredPoint = mHoveredPoint_d;
        const auto lastNSegment = mHoveredNormalSegment;

        updateHovered(e);
        return;
    }

    if(mCurrentMode == CanvasMode::paint && leftPressed)  {
        mPaintTarget.paintMove(e.fPos, e.fTimestamp, 1,
                               0, 0, mDocument.fBrush);
        return;
    } else if(mCurrentMode == CanvasMode::sculptPath && leftPressed) {
        return sculptMove(e.fPos, 1);
    } else if(leftPressed || e.fMouseGrabbing) {
        if(mMovesToSkip > 0) {
            mMovesToSkip--;
            return;
        }
        if(mStartTransform && leftPressed) {
            if((mCurrentMode == CanvasMode::pointTransform &&
                !mPressedPoint && !mCurrentNormalSegment.isValid()) ||
               (mCurrentMode == CanvasMode::boxTransform &&
                !mPressedBox && !mPressedPoint)) {
                startSelectionAtPoint(e.fPos);
            }
        }
        if(mSelecting) {
            moveSecondSelectionPoint(e.fPos);
        } else if(mCurrentMode == CanvasMode::pointTransform) {
            handleMovePointMouseMove(e);
        } else if(mCurrentMode == CanvasMode::boxTransform) {
            if(mPressedPoint) {
                handleMovePointMouseMove(e);
            } else {
                handleMovePathMouseMove(e);
            }
        } else if(mCurrentMode == CanvasMode::pathCreate) {
            handleAddSmartPointMouseMove(e);
        } else if(mCurrentMode == CanvasMode::circleCreate) {
            if(e.shiftMod()) {
                const qreal lenR = pointToLen(e.fPos - e.fLastPressPos);
                mCurrentCircle->moveRadiusesByAbs({lenR, lenR});
            } else {
                mCurrentCircle->moveRadiusesByAbs(e.fPos - e.fLastPressPos);
            }
        } else if(mCurrentMode == CanvasMode::rectCreate) {
            if(e.shiftMod()) {
                const QPointF trans = e.fPos - e.fLastPressPos;
                const qreal valF = qMax(trans.x(), trans.y());
                mCurrentRectangle->moveSizePointByAbs({valF, valF});
            } else {
                mCurrentRectangle->moveSizePointByAbs(e.fPos - e.fLastPressPos);
            }
        }
    }
    mStartTransform = false;

    if(!mSelecting && !e.fMouseGrabbing && leftPressed)
        e.fGrabMouse();
}

void Canvas::mouseReleaseEvent(const MouseEvent &e) {
    if(isPreviewingOrRendering()) return;
    if(e.fButton != Qt::LeftButton) return;
    schedulePivotUpdate();
    if(mCurrentMode == CanvasMode::paint)
        return mPaintTarget.paintRelease();
    else if(mCurrentMode == CanvasMode::sculptPath)
        return sculptRelease(e.fPos, 1);

    handleLeftMouseRelease(e);

    mPressedBox = nullptr;
    mHoveredPoint_d = mPressedPoint;
    mPressedPoint = nullptr;
}

#include "MovablePoints/smartnodepoint.h"
#include "MovablePoints/pathpointshandler.h"
#include "Private/document.h"
void Canvas::mouseDoubleClickEvent(const MouseEvent &e) {
    if(e.fModifiers & Qt::ShiftModifier) return;
    mDoubleClick = true;

    if(mHoveredPoint_d) {
        if(mCurrentMode == CanvasMode::pointTransform &&
           mHoveredPoint_d->isSmartNodePoint()) {
            const auto adder = [this](MovablePoint* const pt) {
                addPointToSelection(pt);
            };
            const auto node = static_cast<SmartNodePoint*>(mHoveredPoint_d.data());
            node->getHandler()->addAllPointsToSelection(adder, mCurrentMode);
        }
    } else if(mHoveredBox) {
        if(mHoveredBox->SWT_isContainerBox() && !mHoveredBox->SWT_isLinkBox()) {
            setCurrentBoxesGroup(static_cast<ContainerBox*>(mHoveredBox.data()));
            updateHovered(e);
        } else if((mCurrentMode == CanvasMode::boxTransform ||
                   mCurrentMode == CanvasMode::pointTransform) &&
                  mHoveredBox->SWT_isTextBox()) {
            e.fReleaseMouse();
            static_cast<TextBox*>(mHoveredBox.data())->openTextEditor(e.fWidget);
        } else if(mCurrentMode == CanvasMode::boxTransform &&
                  mHoveredBox->SWT_isSmartVectorPath()) {
            Document::sInstance->setCanvasMode(CanvasMode::pointTransform);
        }
    } else if(!mHoveredBox && !mHoveredPoint_d && !mHoveredNormalSegment.isValid()) {
        if(mCurrentContainer != this) {
            setCurrentBoxesGroup(mCurrentContainer->getParentGroup());
        }
    }
}

void Canvas::sculptPress(const QPointF& pos, const qreal pressure) {
    auto& absSculptBrush = mDocument.fSculptBrush;
    absSculptBrush.pressAt(pos, pressure);
    if(!hasValidSculptTarget()) {
        const auto newBox = newSculptPathBox(pos);
        const qreal radius = absSculptBrush.radius();
        newBox->setPath(SkPath().addCircle(0, 0, radius));
    }
    for(const auto& box : mSelectedBoxes) {
        if(!box->SWT_isSculptPathBox()) continue;
        const auto sculptBox = static_cast<SculptPathBox*>(box);
        const auto transform = sculptBox->getTotalTransform();
        const auto relBrush = SculptBrush(transform, absSculptBrush);
        sculptBox->sculptStarted();
        sculptBox->sculpt(mDocument.fSculptTarget,
                          mDocument.fSculptMode,
                          relBrush);
    }
    if(mDocument.fSculptTarget == SculptTarget::position &&
       mDocument.fSculptMode == SculptMode::add) {
        mDocument.setSculptMode(SculptMode::drag);
    }
}

void Canvas::sculptMove(const QPointF& pos, const qreal pressure) {
    mDocument.fSculptBrush.moveTo(pos, pressure);
    for(const auto& box : mSelectedBoxes) {
        if(!box->SWT_isSculptPathBox()) continue;
        const auto sculptBox = static_cast<SculptPathBox*>(box);
        const auto transform = sculptBox->getTotalTransform();
        const auto relBrush = SculptBrush(transform.inverted(),
                                          mDocument.fSculptBrush);
        sculptBox->sculpt(mDocument.fSculptTarget,
                          mDocument.fSculptMode,
                          relBrush);
    }
}

void Canvas::sculptCancel() {
    for(const auto& box : mSelectedBoxes) {
        if(!box->SWT_isSculptPathBox()) continue;
        const auto sculptBox = static_cast<SculptPathBox*>(box);
        sculptBox->sculptCanceled();
    }
}

void Canvas::sculptRelease(const QPointF& pos, const qreal pressure) {
    Q_UNUSED(pos)
    Q_UNUSED(pressure)
    for(const auto& box : mSelectedBoxes) {
        if(!box->SWT_isSculptPathBox()) continue;
        const auto sculptBox = static_cast<SculptPathBox*>(box);
        sculptBox->sculptFinished();
    }
}

void Canvas::tabletEvent(const QTabletEvent * const e,
                         const QPointF &pos) {
    const auto type = e->type();
    if(mCurrentMode == CanvasMode::paint) {
        if(type == QEvent::TabletRelease ||
           e->buttons() & Qt::MiddleButton) {
            mStylusDrawing = false;
            mPaintTarget.paintRelease();
        } else if(e->type() == QEvent::TabletPress) {
            if(e->button() == Qt::RightButton) return;
            if(e->button() == Qt::LeftButton) {
                mStylusDrawing = true;
                if(!mPaintTarget.isValid()) newPaintBox(pos);
                mPaintTarget.paintPress(pos, e->timestamp(), e->pressure(),
                                        e->xTilt(), e->yTilt(),
                                        mDocument.fBrush);
            }
        } else if(type == QEvent::TabletMove && mStylusDrawing) {
            mPaintTarget.paintMove(pos, e->timestamp(), e->pressure(),
                                   e->xTilt(), e->yTilt(),
                                   mDocument.fBrush);
        }
    } else if(mCurrentMode == CanvasMode::sculptPath) {
        if(type == QEvent::TabletRelease ||
           e->buttons() & Qt::MiddleButton) {
            mStylusDrawing = false;
            sculptRelease(pos, e->pressure());
        } else if(e->type() == QEvent::TabletPress) {
            if(e->button() == Qt::RightButton) return;
            if(e->button() == Qt::LeftButton) {
                mStylusDrawing = true;
                sculptPress(pos, e->pressure());
            }
        } else if(type == QEvent::TabletMove && mStylusDrawing) {
            sculptMove(pos, e->pressure());
        }
    }
}
