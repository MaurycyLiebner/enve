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
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
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
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
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
