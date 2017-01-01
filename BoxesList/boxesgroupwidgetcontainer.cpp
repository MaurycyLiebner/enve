#include "boxesgroupwidgetcontainer.h"
#include "boxesgroup.h"
#include "boxeslistwidget.h"
#include "boxitemwidget.h"

BoxesGroupWidgetContainer::BoxesGroupWidgetContainer(BoxesGroup *target,
                                                     QWidget *parent) :
    BoxItemWidgetContainer(target, parent) {

}

BoxesGroupWidgetContainer *BoxesGroupWidgetContainer::
    createBoxesGroupWidgetContainer(BoxesGroup *target,
                                    QWidget *parent) {
    BoxesGroupWidgetContainer *newWidget =
            new BoxesGroupWidgetContainer(target, parent);
    newWidget->initialize();
    return newWidget;
}

void BoxesGroupWidgetContainer::initialize() {
    setContentsMargins(0, 0, 0, 0);
    mAllLayout = new QVBoxLayout(this);
    mAllLayout->setAlignment(Qt::AlignTop);
    mAllLayout->setSpacing(0);
    mAllLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mAllLayout);

    mDetailsWidget = new QWidget(this);

    mChildWidgetsLayout = new QVBoxLayout();

    mDetailsLayout = new QVBoxLayout(mDetailsWidget);
    mDetailsLayout->setAlignment(Qt::AlignTop);
    mDetailsLayout->setSpacing(0);
    mDetailsLayout->setContentsMargins(
                BoxesListWidget::getListItemChildIndent(), 0, 0, 0);

    mChildBoxesLayout = new QVBoxLayout();
    mChildBoxesLayout->setAlignment(Qt::AlignTop);
    mChildBoxesLayout->setSpacing(0);
    mChildBoxesLayout->setContentsMargins(0, 0, 0, 0);

    mChildWidgetsLayout->setAlignment(Qt::AlignTop);
    mChildWidgetsLayout->setSpacing(0);
    mChildWidgetsLayout->setContentsMargins(0, 0, 0, 0);

    mDetailsLayout->addLayout(mChildWidgetsLayout);
    mDetailsLayout->addLayout(mChildBoxesLayout);
    mDetailsWidget->setLayout(mDetailsLayout);

    mAllLayout->addWidget(mDetailsWidget);

    mDetailsWidget->hide();

    setTopWidget(mTargetBoxWidget);
    connect(mTargetBoxWidget, SIGNAL(detailsVisibilityChanged(bool)),
            mDetailsWidget, SLOT(setVisible(bool)));

    BoundingBox *target = mTargetBoxWidget->getTargetBox();
    target->addAllAnimatorsToBoxItemWidgetContainer(this);

    connect(target, SIGNAL(addActiveAnimatorSignal(QrealAnimator*)),
            this, SLOT(addAnimatorWidgetForAnimator(QrealAnimator*)));
    connect(target, SIGNAL(removeActiveAnimatorSignal(QrealAnimator*)),
            this, SLOT(removeAnimatorWidgetForAnimator(QrealAnimator*)));
}

void BoxesGroupWidgetContainer::addWidgetForChildBox(BoundingBox *box)
{
    BoxItemWidgetContainer *boxContainer;
    if(box->isGroup()) {
        BoxesGroup *group = (BoxesGroup*)box;
        boxContainer = createBoxesGroupWidgetContainer(group,
                                                       this);
        connect(group, &BoxesGroup::addAnimatedBoundingBoxSignal,
                (BoxesGroupWidgetContainer*)boxContainer,
                &BoxesGroupWidgetContainer::addWidgetForChildBox);
        connect(group, &BoxesGroup::removeAnimatedBoundingBoxSignal,
                (BoxesGroupWidgetContainer*)boxContainer,
                &BoxesGroupWidgetContainer::removeWidgetForChildBox);
    } else {
        boxContainer =
                BoxItemWidgetContainer::createBoxItemWidgetContainer(box, this);
    }
    mChildBoxesLayout->addWidget(boxContainer);
    mBoxWidgetsList << boxContainer;
}

void BoxesGroupWidgetContainer::removeWidgetForChildBox(BoundingBox *box)
{

}
