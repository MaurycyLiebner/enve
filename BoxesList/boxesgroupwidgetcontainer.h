#ifndef BOXESGROUPWIDGETCONTAINER_H
#define BOXESGROUPWIDGETCONTAINER_H

#include "boxitemwidgetcontainer.h"
class BoxesGroup;

class BoxesGroupWidgetContainer : public BoxItemWidgetContainer
{
    Q_OBJECT
public:
    BoxesGroupWidgetContainer(BoxesGroup *target, QWidget *parent = 0);
protected:
//    QVBoxLayout *mDetailsLayout;
//    QVBoxLayout *mChildBoxesLayout;
    QList<BoxItemWidgetContainer*> mBoxWidgetsList;
signals:

public slots:
    void addWidgetForChildBox(BoundingBox *box);
    void removeWidgetForChildBox(BoundingBox *box);
};

#endif // BOXESGROUPWIDGETCONTAINER_H
