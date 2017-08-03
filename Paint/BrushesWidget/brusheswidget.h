#ifndef BRUSHESWIDGET_H
#define BRUSHESWIDGET_H

#include <QGraphicsView>

class BrushSelector;
class BrushCollectionSelector;

struct WindowVariables;

class BrushesWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit BrushesWidget(WindowVariables *window_vars_t, QWidget *parent = 0);
    void resizeEvent(QResizeEvent *e);
signals:

public slots:
private:
    WindowVariables *window_vars = NULL;
    QGraphicsScene *graphics_scene = NULL;
    BrushSelector *brush_selector = NULL;
    BrushCollectionSelector *brush_collection_selector = NULL;
};

#endif // BRUSHESWIDGET_H
