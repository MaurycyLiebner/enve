#include "brushselectionicon.h"
#include <QDialog>

BrushSelectionIcon::BrushSelectionIcon(QWidget *parent) : QWidget(parent) {
    setFixedSize(64, 64);
    mSelectionWidget = new BrushSelectionWidget(this);
    mSelectionWidget->hide();
    connect(mSelectionWidget, &BrushSelectionWidget::currentBrushChanged,
            this, &BrushSelectionIcon::brushSelected);
    mSelectionWidget->setFirstItem();
}

void BrushSelectionIcon::openDialog() {
    mSelectionWidget->show();
    QDialog dialog;
    dialog.setLayout(new QVBoxLayout);
    dialog.layout()->addWidget(mSelectionWidget);
    dialog.setMinimumSize(300, 350);
//    connect(mSelectionWidget, &BrushSelectionWidget::currentBrushChanged,
//            &dialog, &QDialog::close);
    dialog.exec();
    mSelectionWidget->setParent(this);
    mSelectionWidget->hide();
}

void BrushSelectionIcon::brushSelected(BrushWrapper * const wrapper) {
    if(mIcon) delete mIcon;
    mIcon = nullptr;
    mIcon = BrushWidget::createWidget(GetAsSPtr(wrapper, BrushWrapper), this);
    mIcon->show();
    connect(mIcon, &BrushWidget::leftPressed,
            this, &BrushSelectionIcon::openDialog);
    currentBrushChanged(wrapper);
}
