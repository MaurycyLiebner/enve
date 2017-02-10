#include "boxsinglewidget.h"
#include "OptimalScrollArea/singlewidgetabstraction.h"
#include "OptimalScrollArea/singlewidgettarget.h"
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"

#include "Boxes/boxesgroup.h"
#include "qrealanimatorvalueslider.h"

BoxSingleWidget::BoxSingleWidget(ScrollWidgetVisiblePart *parent) :
    SingleWidget(parent) {
    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    mMainLayout->setMargin(0);
    mMainLayout->setAlignment(Qt::AlignLeft);

    mContentButton = new QPushButton(this);
    mMainLayout->addWidget(mContentButton);
    connect(mContentButton, SIGNAL(pressed()),
            this, SLOT(switchContentVisibleAction()));
    mContentButton->setFixedWidth(20);

    mNameLabel = new QLabel(this);
    mMainLayout->addWidget(mNameLabel);

    mValueSlider = new QrealAnimatorValueSlider(NULL, this);
    mMainLayout->addWidget(mValueSlider, Qt::AlignRight);
}

void BoxSingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    SingleWidget::setTargetAbstraction(abs);
    SingleWidgetTarget *target = abs->getTarget();
    const SingleWidgetTargetType &type = target->SWT_getType();

    if(type == SWT_BoundingBox) {
        BoundingBox *bb_target = (BoundingBox*)target;

        setName(bb_target->getName());

        mContentButton->show();
        mContentButton->setChecked(abs->contentVisible());

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(type == SWT_BoxesGroup) {
        BoxesGroup *bg_target = (BoxesGroup*)target;

        setName(bg_target->getName());

        mContentButton->show();
        mContentButton->setChecked(abs->contentVisible());

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(type == SWT_QrealAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;

        setName(qa_target->getName());
        mContentButton->hide();

        mValueSlider->setAnimator(qa_target);
        mValueSlider->show();
    } else if(type == SWT_ComplexAnimator) {
        ComplexAnimator *ca_target = (ComplexAnimator*)target;

        setName(ca_target->getName());

        mContentButton->show();
        mContentButton->setChecked(abs->contentVisible());

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    }
}

void BoxSingleWidget::setName(const QString &name) {
    mNameLabel->setText(name);
}

void BoxSingleWidget::switchContentVisibleAction() {
    mTarget->switchContentVisible();
    mParent->callUpdaters();
}
