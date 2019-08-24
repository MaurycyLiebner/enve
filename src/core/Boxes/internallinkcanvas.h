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

#ifndef INTERNALLINKCANVAS_H
#define INTERNALLINKCANVAS_H
#include "internallinkgroupbox.h"
#include "Properties/boolproperty.h"

class InternalLinkCanvas : public InternalLinkGroupBox {
    e_OBJECT
protected:
    InternalLinkCanvas(ContainerBox * const linkTarget);
public:
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoxRenderData> createRenderData();

    bool relPointInsidePath(const QPointF &relPos) const;
    void anim_setAbsFrame(const int frame);

    bool clipToCanvas();
private:
    qsptr<BoolProperty> mClipToCanvas =
            enve::make_shared<BoolProperty>("clip");
};

#endif // INTERNALLINKCANVAS_H
