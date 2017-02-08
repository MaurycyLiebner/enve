#include "boxsinglewidget.h"
#include "OptimalScrollArea/singlewidgetabstraction.h"
#include "OptimalScrollArea/singlewidgettarget.h"

#include "Boxes/boxesgroup.h"

BoxSingleWidget::BoxSingleWidget(QWidget *parent) :
    SingleWidget(parent) {
    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    mMainLayout->setMargin(0);
    mMainLayout->setAlignment(Qt::AlignLeft);

    mNameLabel = new QLabel(this);
    mMainLayout->addWidget(mNameLabel);
}

void BoxSingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    SingleWidget::setTargetAbstraction(abs);
    SingleWidgetTarget *target = abs->getTarget();
    const SingleWidgetTargetType &type = target->SWT_getType();

    if(type == SWT_BoundingBox) {
        BoundingBox *bb_target = (BoundingBox*)target;

        setName(bb_target->getName());
    } else if(type == SWT_BoxesGroup) {
        BoxesGroup *bg_target = (BoxesGroup*)target;

        setName(bg_target->getName());
    } else if(type == SWT_QrealAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;

        setName(qa_target->getName());
    } else if(type == SWT_ComplexAnimator) {
        ComplexAnimator *ca_target = (ComplexAnimator*)target;

        setName(ca_target->getName());
    }
}

void BoxSingleWidget::setName(const QString &name) {
    mNameLabel->setText(name);
}
