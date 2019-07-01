#ifndef TIMELINEWRAPPER_H
#define TIMELINEWRAPPER_H
#include "canvaswindowwrapper.h"
class TimelineWidget;
class ChangeWidthWidget;

struct TWidgetStackLayoutItem : public SceneWidgetStackLayoutItem {
    TWidgetStackLayoutItem() {}
    void clear();
    QWidget* create(Document &document, AudioHandler &audioHandler,
                    QWidget * const parent, QLayout* const layout = nullptr);
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
    void setGraph(const bool graph);
private:
    bool mGraph = false;
};

class TimelineWrapper : public StackWidgetWrapper {
public:
    TimelineWrapper(Document * const document,
                    TWidgetStackLayoutItem * const layItem,
                    QWidget * const parent = nullptr);

    void setScene(Canvas* const scene);
    Canvas* getScene() const;
    TimelineWidget *getTimelineWidget() const;

    void saveDataToLayout() const;
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

#endif // TIMELINEWRAPPER_H
