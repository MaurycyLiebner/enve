#include "settingswidget.h"

#include "Private/esettings.h"

#include <QVBoxLayout>
#include <QFrame>

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent), mSett(*eSettings::sInstance) {
    mMainLauout = new QVBoxLayout;
    mMainLauout->setAlignment(Qt::AlignTop);
    setLayout(mMainLauout);
}

void SettingsWidget::add2HWidgets(QWidget * const widget1,
                                  QWidget * const widget2) {
    const auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget1);
    layout->addWidget(widget2);
    addLayout(layout);
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
