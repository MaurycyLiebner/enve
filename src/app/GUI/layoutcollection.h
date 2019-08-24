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

#ifndef LAYOUTCOLLECTION_H
#define LAYOUTCOLLECTION_H
#include "canvas.h"
#include "canvaswindowwrapper.h"
#include "timelinewrapper.h"

struct SceneBaseStackItem : public BaseStackItem {
    typedef std::unique_ptr<SceneBaseStackItem> cUPtr;
protected:
    SceneBaseStackItem(std::unique_ptr<SceneWidgetStackLayoutItem>&& cwwItem,
                       Canvas* const scene = nullptr) : mScene(scene) {
        if(scene) setName(scene->prp_getName());
        cwwItem->setScene(scene);
        setChild(std::move(cwwItem));
    }

    template <typename WidgetT>
    void readSceneBaseStackItem(eReadStream& src) {
        readBaseStackItem<WidgetT>(src);
    }
public:
    void setScene(Canvas* const scene) {
        mScene = scene;
    }

    Canvas* getScene() const { return mScene; }
private:
    Canvas* mScene = nullptr;
};

struct TSceneBaseStackItem : public SceneBaseStackItem {
    TSceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(std::make_unique<TWidgetStackLayoutItem>(), scene) {}

    void read(eReadStream& src) {
        readSceneBaseStackItem<TWidgetStackLayoutItem>(src);
    }
};

struct CWSceneBaseStackItem : public SceneBaseStackItem {
    CWSceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(std::make_unique<CWWidgetStackLayoutItem>(), scene) {}

    void read(eReadStream& src) {
        readSceneBaseStackItem<CWWidgetStackLayoutItem>(src);
    }
};

#endif // LAYOUTCOLLECTION_H
