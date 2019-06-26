#ifndef CANVASWINDOWWRAPPER_H
#define CANVASWINDOWWRAPPER_H
#include "stackwidgetwrapper.h"
class Document;
class Canvas;
class CanvasWindow;

struct SceneWidgetStackLayoutItem : public WidgetStackLayoutItem {
    void clear();
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
    void setScene(Canvas* const scene);
protected:
    Canvas* mScene = nullptr;
};

struct CWWidgetStackLayoutItem : public SceneWidgetStackLayoutItem {
    void clear();
    void apply(StackWidgetWrapper* const stack) const;
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
    void setTransform(const QMatrix& transform);
private:
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
protected:
    void changeEvent(QEvent* e);
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

#endif // CANVASWINDOWWRAPPER_H
