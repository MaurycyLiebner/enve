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

#include "paintbox.h"
#include "canvas.h"
#include "MovablePoints/animatedpoint.h"
#include "Animators/transformanimator.h"

PaintBox::PaintBox() : BoundingBox(eBoxType::paint) {
    prp_setName("Paint Box");
    mSurface = enve::make_shared<AnimatedSurface>();
    ca_addChild(mSurface);
}

void PaintBox::setupRenderData(const qreal relFrame,
                               BoxRenderData * const data,
                               Canvas* const scene) {
    BoundingBox::setupRenderData(relFrame, data, scene);
    const auto paintData = static_cast<PaintBoxRenderData*>(data);
    const int imgFrame = qFloor(relFrame);
    const auto imgTask = mSurface->getFrameImage(imgFrame, paintData->fImage);
    if(imgTask) imgTask->addDependent(data);

    paintData->fASurface = mSurface.get();
    paintData->fSurface = enve::shared(mSurface->getSurface(imgFrame));
}

stdsptr<BoxRenderData> PaintBox::createRenderData() {
    return enve::make_shared<PaintBoxRenderData>(this);
}

#include "GUI/edialogs.h"
#include "typemenu.h"
#include "filesourcescache.h"
void PaintBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<PaintBox>()) return;
    menu->addedActionsForType<PaintBox>();
    PropertyMenu::PlainSelectedOp<PaintBox> op = [](PaintBox * box) {
        const QString filters = FileExtensions::imageFilters();
        const QString importPath = eDialogs::openFile(
                                        "Load From Image", QDir::homePath(),
                                        "Image Files (" + filters + ")");
        if(!importPath.isEmpty()) {
            QImage img;
            if(img.load(importPath)) {
                //box->loadFromImage(img);
            }
        }
    };
    menu->addPlainAction("Load From Image", op);

    BoundingBox::setupCanvasMenu(menu);
}
