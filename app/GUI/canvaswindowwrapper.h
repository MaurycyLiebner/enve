#ifndef CANVASWINDOWWRAPPER_H
#define CANVASWINDOWWRAPPER_H
#include "stackwidgetwrapper.h"
class Document;
class Canvas;
struct CWWidgetStackLayoutItem;

class CanvasWindowWrapper : public StackWidgetWrapper {
public:
    CanvasWindowWrapper(Document * const document,
                        CWWidgetStackLayoutItem * const layItem,
                        QWidget * const parent = nullptr);
    void setScene(Canvas* const scene);
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

struct CWWidgetStackLayoutItem : public WidgetStackLayoutItem {
    void clear();
    void apply(StackWidgetWrapper* const stack) const;
    void setScene(Canvas* const scene);
private:
    Canvas* mScene = nullptr;
};


#endif // CANVASWINDOWWRAPPER_H
