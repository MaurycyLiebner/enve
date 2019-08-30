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

#ifndef OUTLINESETTINGSANIMATOR_H
#define OUTLINESETTINGSANIMATOR_H
#include "paintsettingsanimator.h"

class OutlineSettingsAnimator : public PaintSettingsAnimator {
    e_OBJECT
protected:
    OutlineSettingsAnimator(GradientPoints* const grdPts,
                            PathBox * const parent);
public:
    bool SWT_isStrokeSettings() const { return true; }
    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);
public:
    void strokeWidthAction(const QrealAction& action);
    void setCurrentStrokeWidth(const qreal newWidth);
    void setCapStyle(const SkPaint::Cap capStyle);
    void setJoinStyle(const SkPaint::Join joinStyle);
    void setStrokerSettingsSk(SkStroke * const stroker);

    qreal getCurrentStrokeWidth() const;

    SkPaint::Cap getCapStyle() const;
    SkPaint::Join getJoinStyle() const;

    QrealAnimator *getStrokeWidthAnimator();

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    QPainter::CompositionMode getOutlineCompositionMode();
    void setLineWidthUpdaterTarget(PathBox * const path);
    bool nonZeroLineWidth();

    QrealAnimator *getLineWidthAnimator();

    void setStrokerSettingsForRelFrameSk(const qreal relFrame,
                                         SkStroke * const stroker);
private:
    SkPaint::Cap mCapStyle = SkPaint::kRound_Cap;
    SkPaint::Join mJoinStyle = SkPaint::kRound_Join;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qsptr<QrealAnimator> mLineWidth =
            enve::make_shared<QrealAnimator>(1, 0, 999, 1, "thickness");
};
#endif // OUTLINESETTINGSANIMATOR_H
