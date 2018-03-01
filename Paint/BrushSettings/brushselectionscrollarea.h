#ifndef BRUSHSELECTIONSCROLLAREA_H
#define BRUSHSELECTIONSCROLLAREA_H
#include "BoxesList/OptimalScrollArea/scrollarea.h"
class Brush;
class BrushSelectionWidget;

class BrushSelectionScrollArea : public ScrollArea {
    Q_OBJECT
public:
    BrushSelectionScrollArea(QWidget *parent = NULL);
    const Brush *getCurrentBrush();
    void saveBrushesForProject(QIODevice *target);
    void readBrushesForProject(QIODevice *target);
signals:
    void brushSelected(const Brush*);
    void brushReplaced(const Brush*, const Brush*); // second one is new
public slots:
    void setCurrentBrush(const Brush *brush);
private:
    BrushSelectionWidget *mBrushesWidget = NULL;
};

#endif // BRUSHSELECTIONSCROLLAREA_H
