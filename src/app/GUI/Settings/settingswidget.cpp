#include "settingswidget.h"

#include <QVBoxLayout>
#include <QFrame>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent) {
    mMainLauout = new QVBoxLayout;
    setLayout(mMainLauout);
}

void SettingsWidget::addWidget(QWidget * const widget) {
    mMainLauout->addWidget(widget);
}

void SettingsWidget::addLayout(QLayout * const layout) {
    mMainLauout->addLayout(layout);
}

void SettingsWidget::addSeparator() {
    const auto line0 = new QFrame;
    line0->setFrameShape(QFrame::HLine);
    line0->setFrameShadow(QFrame::Sunken);
    layout()->addWidget(line0);
}
