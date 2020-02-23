// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "externalappssettingswidget.h"

#include "Private/esettings.h"
#include "GUI/edialogs.h"

#include <QLineEdit>
#include <QLabel>
#include <QAction>
#include <QPushButton>

class AppLineEdit : public QHBoxLayout {
public:
    AppLineEdit(const QString& name) : QHBoxLayout() {
        mLineEdit = new QLineEdit();
        const auto button = new QPushButton("...");
        addWidget(mLineEdit);
        addWidget(button);

        connect(button, &QPushButton::pressed, this, [this, name]() {
            const QString path = eDialogs::openFile("Set " + name + " Path",
                                                    QDir::homePath(), "");
            if(!path.isEmpty()) setText(path);
        });
    }

    QString text() const { return mLineEdit->text(); }
    void setText(const QString& text) const { mLineEdit->setText(text); }
private:
    QLineEdit* mLineEdit;
};

ExternalAppsSettingsWidget::ExternalAppsSettingsWidget(QWidget *parent) :
    SettingsWidget(parent) {
    const auto layout = new QHBoxLayout();
    const auto labelLayout = new QVBoxLayout();
    const auto editLayout = new QVBoxLayout();

    mGimp = new AppLineEdit("Gimp");
    mMyPaint = new AppLineEdit("MyPaint");
    mKrita = new AppLineEdit("Krita");

    labelLayout->addWidget(new QLabel(   "Gimp    ", this), 0, Qt::AlignRight);
    labelLayout->addWidget(new QLabel("MyPaint    ", this), 0, Qt::AlignRight);
    labelLayout->addWidget(new QLabel(  "Krita    ", this), 0, Qt::AlignRight);

    editLayout->addLayout(mGimp);
    editLayout->addLayout(mMyPaint);
    editLayout->addLayout(mKrita);

    layout->addLayout(labelLayout);
    layout->addLayout(editLayout);
    addLayout(layout);
}

void ExternalAppsSettingsWidget::applySettings() {
    mSett.fGimp = mGimp->text();
    mSett.fMyPaint = mMyPaint->text();
    mSett.fKrita = mKrita->text();
}

void ExternalAppsSettingsWidget::updateSettings() {
    mGimp->setText(mSett.fGimp);
    mMyPaint->setText(mSett.fMyPaint);
    mKrita->setText(mSett.fKrita);
}
