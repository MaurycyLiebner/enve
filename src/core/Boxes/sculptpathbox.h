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

#ifndef SCULPTPATHBOX_H
#define SCULPTPATHBOX_H

#include "boundingbox.h"
#include "Animators/SculptPath/sculptpathanimator.h"
#include "Animators/brushsettingsanimator.h"
#include "Animators/coloranimator.h"
#include "Animators/paintsettingsanimator.h"

class SculptPathBox : public BoundingBox {
public:
    SculptPathBox();

    bool SWT_isSculptPathBox() const { return true; }

    void setStrokeBrush(SimpleBrushWrapper * const brush);
    void strokeWidthAction(const QrealAction &action);

    HardwareSupport hardwareSupport() const
    { return HardwareSupport::cpuOnly; }

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);
    stdsptr<BoxRenderData> createRenderData();
    void setupCanvasMenu(PropertyMenu * const menu);

    void readBoundingBox(eReadStream& src);
    void writeBoundingBox(eWriteStream& dst);

    FillSettingsAnimator *getFillSettings() const
    { return mFillSettings.get(); }

    void setPath(const SkPath& path);
    void sculptStarted();
    void sculpt(const SculptTarget target,
                const SculptMode mode,
                const SculptBrush& brush);
    void sculptCanceled();
    void sculptFinished();

    void applyCurrentTransform();
private:
    stdptr<SimpleBrushWrapper> mBrush;

    qsptr<SculptPathAnimator> mPath;
    qsptr<QrealAnimator> mBrushWidth;

    qsptr<FillSettingsAnimator> mFillSettings;
};

#endif // SCULPTPATHBOX_H
