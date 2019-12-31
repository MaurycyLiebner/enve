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

#ifndef ACTIONS_H
#define ACTIONS_H

#include <QString>

class QDropEvent;

#include "colorhelpers.h"
#include "skia/skiaincludes.h"

class PaintSettingsApplier;
class Document;
class Canvas;
class QrealAction;
class eBoxOrSound;
class ContainerBox;
class ExternalLinkBox;

class Actions : public QObject {
public:
    Actions(Document& document);

    static Actions* sInstance;

    void undoAction() const;
    void redoAction() const;

    void raiseAction() const;
    void lowerAction() const;
    void raiseToTopAction() const;
    void lowerToBottomAction() const;

    void objectsToPathAction() const;
    void strokeToPathAction() const;

    void rotate90CWAction() const;
    void rotate90CCWAction() const;
    void flipHorizontalAction() const;
    void flipVerticalAction() const;

    void pathsUnionAction() const;
    void pathsDifferenceAction() const;
    void pathsIntersectionAction() const;
    void pathsDivisionAction() const;
    void pathsExclusionAction() const;
    void pathsCombineAction() const;
    void pathsBreakApartAction() const;

    void setTextAlignment(const Qt::Alignment alignment) const;
    void setFontFamilyAndStyle(const QString& family,
                               const QString& style) const;
    void setFontSize(const qreal size) const;

    void connectPointsSlot() const;
    void disconnectPointsSlot() const;
    void mergePointsSlot() const;
    void subdivideSegments() const;

    void makePointCtrlsSymmetric() const;
    void makePointCtrlsSmooth() const;
    void makePointCtrlsCorner() const;

    void makeSegmentLine() const;
    void makeSegmentCurve() const;

    void newEmptyPaintFrame();

    void deleteAction() const;
    void copyAction() const;
    void pasteAction() const;
    void cutAction() const;
    void duplicateAction() const;

    void selectAllAction() const;
    void invertSelectionAction() const;
    void clearSelectionAction() const;

    void groupSelectedBoxes() const;
    void ungroupSelectedBoxes() const;

    void startSelectedStrokeColorTransform() const;
    void startSelectedFillColorTransform() const;

    void strokeCapStyleChanged(const SkPaint::Cap capStyle) const;
    void strokeJoinStyleChanged(const SkPaint::Join joinStyle) const;

    void strokeWidthAction(const QrealAction& action) const;

    void applyPaintSettingToSelected(
            const PaintSettingsApplier &setting) const;

    void updateAfterFrameChanged(const int currentFrame) const;

    void setClipToCanvas(const bool clip);

    void setRasterEffectsVisible(const bool bT);
    void setPathEffectsVisible(const bool bT);

    eBoxOrSound* handleDropEvent(QDropEvent * const event,
                                 const QPointF &relDropPos = QPointF(0, 0),
                                 const int frame = 0);
    eBoxOrSound* importFile(const QString &path);
    eBoxOrSound* importFile(const QString &path,
                            ContainerBox * const target,
                            const int insertId = 0,
                            const QPointF &relDropPos = QPointF(0, 0),
                            const int frame = 0);
    ExternalLinkBox *linkFile(const QString &path);

    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setPickPaintSettingsMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();
    void setPaintMode();

    bool smoothChange() const { return mSmoothChange; }
    void startSmoothChange() { mSmoothChange = true; }
    void finishSmoothChange();
private:
    void afterAction() const;

    bool mSmoothChange = false;
    Document& mDocument;
    Canvas* const & mActiveScene;
};

#endif // ACTIONS_H
