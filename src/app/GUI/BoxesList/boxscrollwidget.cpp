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

#include "boxscrollwidget.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "Boxes/boundingbox.h"
#include "boxscroller.h"
#include "GUI/canvaswindow.h"
#include "GUI/mainwindow.h"
#include "Private/document.h"

BoxScrollWidget::BoxScrollWidget(Document &document,
                                 ScrollArea * const parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
    mCoreAbs = document.SWT_createAbstraction(
                mVisiblePartWidget->getUpdateFuncs(),
                mVisiblePartWidget->getId());
}

BoxScrollWidget::~BoxScrollWidget() {
    if(mCoreAbs) mCoreAbs->removeAlongWithAllChildren_k();
}

//void BoxScrollWidget::updateAbstraction() {
//    if(!mMainTarget) {
//        mMainAbstraction = nullptr;
//    } else {
//        mMainAbstraction = mMainTarget->
//                SWT_getAbstractionForWidget(mVisiblePartWidget);
//    }
//    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
//    updateHeight();
//}

BoxScroller *BoxScrollWidget::getVisiblePartWidget() {
    return static_cast<BoxScroller*>(mVisiblePartWidget);
}

void BoxScrollWidget::setCurrentScene(Canvas * const scene) {
    getVisiblePartWidget()->setCurrentScene(scene);
}

void BoxScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new BoxScroller(this);
    mMinimalVisiblePartWidget = mVisiblePartWidget;
}
