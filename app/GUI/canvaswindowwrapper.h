#ifndef CANVASWINDOWWRAPPER_H
#define CANVASWINDOWWRAPPER_H
#include "stackwidgetwrapper.h"
class Document;
class Canvas;
class CanvasWindow;
struct CWWidgetStackLayoutItem;

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

struct CWWidgetStackLayoutItem : public WidgetStackLayoutItem {
    void clear();
    void apply(StackWidgetWrapper* const stack) const;
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
    void setScene(Canvas* const scene);
    void setTransform(const QMatrix& transform);
private:
    QMatrix mTransform;
    Canvas* mScene = nullptr;
};


#endif // CANVASWINDOWWRAPPER_H
