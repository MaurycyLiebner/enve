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
#include "Private/esettings.h"
#include "Private/document.h"
#include "Ora/oracreator.h"
#include "Ora/oraparser.h"
#include "Paint/externalpaintapphandler.h"
#include "svgexporter.h"

PaintBox::PaintBox() : BoundingBox("Paint Box", eBoxType::paint) {
    mSurface = enve::make_shared<AnimatedSurface>();
    ca_addChild(mSurface);
}

struct PaintBoxRenderData : public ImageRenderData {
    e_OBJECT
public:
    PaintBoxRenderData(BoundingBox * const parentBoxT) :
        ImageRenderData(parentBoxT) {}

    void loadImageFromHandler() {
        if(fImage) return;
        if(fASurface) fASurface->getFrameImage(qFloor(fRelFrame), fImage);
    }

    void updateRelBoundingRect() final {
        Q_ASSERT(fSurface);
        fRelBoundingRect = fSurface->surface().pixelBoundingRect();
    }

    qptr<AnimatedSurface> fASurface;
    stdsptr<DrawableAutoTiledSurface> fSurface;
};

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

    PropertyMenu::PlainSelectedOp<PaintBox> loadOp = [](PaintBox * box) {
        const QString filters = FileExtensions::imageFilters();
        const QString importPath = eDialogs::openFile(
                                        "Load From Image", QDir::homePath(),
                                        "Image Files (" + filters + ")");
        if(!importPath.isEmpty()) {
            QImage img;
            if(img.load(importPath)) {
                box->mSurface->loadPixmap(img);
            }
        }
    };
    menu->addPlainAction("Load From Image", loadOp);

    const auto editMenu = menu->addMenu("Edit");

    const auto editOp = [](PaintBox * box, const ExternalPaintAppHandler::App app) {
        if(!box) return;
        const auto handler = new ExternalPaintAppHandler(app);
        const auto aSurface = box->getSurface();
        QString layerName = box->prp_getName();
        layerName.replace(' ', '_');
        handler->setupFor(aSurface, layerName);
    };

    const auto settings = eSettings::sInstance;
    if(!settings->fGimp.isEmpty()) {
        PropertyMenu::PlainSelectedOp<PaintBox> gimpOp = [editOp](PaintBox * box) {
            editOp(box, ExternalPaintAppHandler::gimp);
        };
        editMenu->addPlainAction("Gimp", gimpOp);
    }

    if(!settings->fMyPaint.isEmpty()) {
        PropertyMenu::PlainSelectedOp<PaintBox> mypaintOp = [editOp](PaintBox * box) {
            editOp(box, ExternalPaintAppHandler::mypaint);
        };
        editMenu->addPlainAction("MyPaint", mypaintOp);
    }

    if(!settings->fKrita.isEmpty()) {
        PropertyMenu::PlainSelectedOp<PaintBox> kritaOp = [editOp](PaintBox * box) {
            editOp(box, ExternalPaintAppHandler::krita);
        };
        editMenu->addPlainAction("Krita", kritaOp);
    }

    if(editMenu->isEmpty()) editMenu->setVisible(false);

    BoundingBox::setupCanvasMenu(menu);
}

void PaintBox::saveSVG(SvgExporter& exp, DomEleTask* const task) const {
    auto& ele = task->initialize("use");
    const auto visRelRange = prp_absRangeToRelRange(task->visRange());
    const auto pTask = mSurface->savePaintSVG(exp, ele, visRelRange);
    if(pTask) pTask->addDependent(task);
}
