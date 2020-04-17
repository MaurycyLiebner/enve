#include "envelicense.h"

#include "GUI/global.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPushButton>

EnveLicense* EnveLicense::sInstance = nullptr;

EnveLicense::EnveLicense(QWidget * const parent) : QWidget(parent) {
    Q_ASSERT(!sInstance);
    sInstance = this;

    setWindowFlag(Qt::Tool);

    const auto mainLayout = new QVBoxLayout(this);

    const auto licenseText = new QPlainTextEdit();
    QFile licenseFile(":/other/GPL3-license.txt");
    if(licenseFile.open(QIODevice::ReadOnly)) {
        licenseText->setPlainText(licenseFile.readAll());
        licenseFile.close();
    } else licenseText->setPlainText("Could not load license.");
    mainLayout->addWidget(licenseText);
    licenseText->setMinimumWidth(eSizesUI::widget*25);
    licenseText->setMinimumHeight(eSizesUI::widget*10);
    licenseText->setReadOnly(true);

    const auto closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::released,
            this, &EnveLicense::deleteLater);
    closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mainLayout->addWidget(closeButton, 0, Qt::AlignRight);
    setLayout(mainLayout);
}

EnveLicense::~EnveLicense() { sInstance = nullptr; }
