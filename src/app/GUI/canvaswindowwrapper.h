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

#ifndef CANVASWINDOWWRAPPER_H
#define CANVASWINDOWWRAPPER_H
#include "stackwidgetwrapper.h"
#include "stacklayouts.h"
class Document;
class Canvas;
class CanvasWindow;
class AudioHandler;

struct SceneWidgetStackLayoutItem : public WidgetStackLayoutItem {
    void clear();
    void write(eWriteStream& dst) const;
    void read(eReadStream& src);
    void setScene(Canvas* const scene);
protected:
    Canvas* mScene = nullptr;
};

struct CWWidgetStackLayoutItem : public SceneWidgetStackLayoutItem {
    CWWidgetStackLayoutItem() {}

    void clear();
    QWidget* create(Document &document,
                    QWidget * const parent,
                    QLayout* const layout = nullptr);
    void write(eWriteStream& dst) const;
    void read(eReadStream& src);
    void setTransform(const QMatrix& transform);
private:
    bool mTransformSet = false;
    QMatrix mTransform;
};

class CanvasWindowWrapper : public StackWidgetWrapper {
public:
    CanvasWindowWrapper(Document * const document,
                        CWWidgetStackLayoutItem * const layItem,
                        QWidget * const parent = nullptr);

    void setScene(Canvas* const scene);
    Canvas* getScene() const;
    CanvasWindow *getSceneWidget() const;

    void saveDataToLayout() const;
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

#endif // CANVASWINDOWWRAPPER_H
