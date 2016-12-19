#include "boxesgroupwidgetcontainer.h"
#include "boxesgroup.h"
#include "boxeslistwidget.h"

BoxesGroupWidgetContainer::BoxesGroupWidgetContainer(BoxesGroup *target,
                                                     QWidget *parent) :
    BoxItemWidgetContainer(target, parent)
{
//    delete mChildWidgetsLayout;
//    mChildWidgetsLayout = new QVBoxLayout();

//    mDetailsLayout = new QVBoxLayout(mDetailsWidget);
//    mDetailsLayout->setAlignment(Qt::AlignTop);
//    mDetailsLayout->setSpacing(0);
//    mDetailsLayout->setContentsMargins(BoxesListWidget::getListItemChildIndent(), 0, 0, 0);

//    mChildBoxesLayout = new QVBoxLayout();
//    mChildBoxesLayout->setAlignment(Qt::AlignTop);
//    mChildBoxesLayout->setSpacing(0);
//    mChildBoxesLayout->setContentsMargins(0, 0, 0, 0);

//    mChildWidgetsLayout->setAlignment(Qt::AlignTop);
//    mChildWidgetsLayout->setSpacing(0);
//    mChildWidgetsLayout->setContentsMargins(0, 0, 0, 0);

//    mDetailsLayout->addLayout(mChildWidgetsLayout);
//    mDetailsLayout->addLayout(mChildBoxesLayout);
//    mDetailsWidget->setLayout(mDetailsLayout);
}

void BoxesGroupWidgetContainer::addWidgetForChildBox(BoundingBox *box)
{
    BoxItemWidgetContainer *boxContainer;
    if(box->isGroup()) {
        BoxesGroup *group = (BoxesGroup*)box;
        boxContainer = new BoxesGroupWidgetContainer(group,
                                                     this);
        connect(group, &BoxesGroup::addBoundingBoxSignal,
                (BoxesGroupWidgetContainer*)boxContainer,
                &BoxesGroupWidgetContainer::addWidgetForChildBox);
        connect(group, &BoxesGroup::removeBoundingBoxSignal,
                (BoxesGroupWidgetContainer*)boxContainer,
                &BoxesGroupWidgetContainer::removeWidgetForChildBox);
    } else {
        boxContainer = new BoxItemWidgetContainer(box, this);
    }
    mChildWidgetsLayout->addWidget(boxContainer);
    mBoxWidgetsList << boxContainer;
}

void BoxesGroupWidgetContainer::removeWidgetForChildBox(BoundingBox *box)
{

}
