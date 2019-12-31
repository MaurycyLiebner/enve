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

#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H
#include "Animators/eeffect.h"
#include "Animators/qrealanimator.h"
#include "Properties/comboboxproperty.h"
#include "Animators/transformanimator.h"
#include "Animators/rastereffectanimators.h"
#include "PathEffects/patheffectanimators.h"

class LetterRenderData;
class WordRenderData;
class LineRenderData;
class TextBoxRenderData;

enum class TextFragmentType : short;

class TextEffect : public eEffect {
    Q_OBJECT
public:
    TextEffect();

    bool SWT_dropSupport(const QMimeData * const data);
    bool SWT_drop(const QMimeData * const data);
    QMimeData *SWT_createMimeData();

    void prp_setupTreeViewMenu(PropertyMenu * const menu);
    void prp_drawCanvasControls(SkCanvas * const canvas,
                                const CanvasMode mode,
                                const float invScale,
                                const bool ctrlPressed);

    void writeIdentifier(eWriteStream& dst) const
    { Q_UNUSED(dst) }

    void apply(TextBoxRenderData * const textData) const;
    TextFragmentType target() const;
private:
    QMatrix getTransform(const qreal relFrame,
                         const qreal influence,
                         const QPointF &addPivot) const;
    void applyToLetter(LetterRenderData * const letterData,
                       const qreal influence) const;
    void applyToWord(WordRenderData * const wordData,
                     const qreal influence) const;
    void applyToLine(LineRenderData * const lineData,
                     const qreal influence) const;

    qsptr<QrealAnimator> mInfluence;
    qsptr<ComboBoxProperty> mTarget;
    qsptr<QrealAnimator> mMinInfluence;

    qsptr<StaticComplexAnimator> mDiminishCont;
    qsptr<QrealAnimator> mDiminishInfluence;
    qsptr<QrealAnimator> mDiminishCenter;
    qsptr<QrealAnimator> mDiminishRange;
    qsptr<QrealAnimator> mDiminishSmoothness;

    qsptr<StaticComplexAnimator> mPeriodicCont;
    qsptr<QrealAnimator> mPeriod;
    qsptr<QrealAnimator> mPeriodicInfluence;
    qsptr<QrealAnimator> mPeriodicSmoothness;

    qsptr<AdvancedTransformAnimator> mTransform;

    qsptr<PathEffectAnimators> mBasePathEffects;
    qsptr<PathEffectAnimators> mFillPathEffects;
    qsptr<PathEffectAnimators> mOutlineBasePathEffects;
    qsptr<PathEffectAnimators> mOutlinePathEffects;

    qsptr<RasterEffectAnimators> mRasterEffects;
};

#endif // TEXTEFFECT_H
