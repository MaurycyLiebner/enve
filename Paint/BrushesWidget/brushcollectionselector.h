#ifndef BRUSHCOLLECTIONSELECTOR_H
#define BRUSHCOLLECTIONSELECTOR_H
#include <QGraphicsItem>
#include <QFont>
#include "collectionbutton.h"

#include "brushselector.h"

class BrushCollectionSelector : public QGraphicsItem
{
public:
    BrushCollectionSelector(BrushSelector *brush_selector_t);
    void selectCollection(CollectionButton *current_collection_t);
    void loadCollections(QString from_folder_t);
    int getButtonHeight();
    void setButtonHeight(int height_t);
    QFont getButtonFont();
    void setWidth(int width_t);
    void setHeight(int height_t);
    void updateButtons();
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    int getButtonMargin();
private:
    QFont button_font;
    int height = 0;
    int width = 0;
    int button_height = 0;
    int button_margin = 5;
    CollectionButton *current_collection = NULL;
    BrushSelector *brush_selector = NULL;
    QList<CollectionButton*> collections;
    void generateButtonRowMargins(QList<int> *row_margins,
                                  QList<int> *last_button_margins);
};

#endif // BRUSHCOLLECTIONSELECTOR_H
