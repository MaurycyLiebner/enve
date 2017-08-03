#ifndef COLLECTIONBUTTON_H
#define COLLECTIONBUTTON_H
#include <QGraphicsItem>
#include "brushselector.h"

class BrushCollectionSelector;

class BrushSelector;

struct Collection;

class CollectionButton : public QGraphicsItem
{
public:
    CollectionButton(QString folder_path, BrushSelector *brush_selector_t,
                     BrushCollectionSelector *parent_t);
    void select();
    void deselect();
    void setSelected(bool selected_t);
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    void setAdditionalMargin(int add_margin_t);
    void updateWidth();
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    int getWidth();
    Collection *getCollection();
    bool isDarker();
    void setDarker(bool darker_t);
    void updateTextWidth();
    int getTextWidth();
private:
    int add_margin = 0;
    bool darker = false;
    int width = 0;
    int text_width = 0;
    Collection collection;
    bool hover = false;
    bool selected = false;
    BrushCollectionSelector *brush_collection_selector = NULL;
};

#endif // COLLECTIONBUTTON_H
