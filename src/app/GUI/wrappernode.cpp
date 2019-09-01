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

#include "wrappernode.h"
#include "widgetwrappernode.h"

WrapperNode* WrapperNode::sRead(eReadStream &src,
                                const WrapperNodeCreator &creator) {
    WrapperNodeType type;
    src.read(&type, sizeof(WrapperNodeType));
    WrapperNode* wid = nullptr;
    if(type == WrapperNodeType::widget) {
        wid = creator(nullptr);
    } else if(type == WrapperNodeType::splitH) {
        wid = new HWidgetStackNode(creator);
    } else if(type == WrapperNodeType::splitV) {
        wid = new VWidgetStackNode(creator);
    } else if(type == WrapperNodeType::base) {
        wid = new BaseWrapperNode(creator);
    }
    wid->readData(src);
    return wid;
}

void BaseWrapperNode::reset(Canvas* const scene) {
    if(fChild) delete fChild;
    fChild = fCreator(scene);
    fChild->fParent = this;
    layout()->addWidget(fChild->widget());
}
