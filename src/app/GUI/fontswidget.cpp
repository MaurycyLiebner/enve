// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#include "fontswidget.h"

#include "mainwindow.h"

#include <QLineEdit>
#include <QIntValidator>
#include <QLabel>

FontsWidget::FontsWidget(QWidget *parent) : QWidget(parent) {
    mFontStyleCombo = new QComboBox(this);
    mFontStyleCombo->setFocusPolicy(Qt::NoFocus);
    mFontFamilyCombo = new QComboBox(this);
    mFontFamilyCombo->setFocusPolicy(Qt::NoFocus);
    mFontSizeCombo = new QComboBox(this);
    mFontSizeCombo->setFocusPolicy(Qt::ClickFocus);
    mFontSizeCombo->setEditable(true);
    mFontSizeCombo->setAutoCompletion(false);
    mFontSizeCombo->setMinimumContentsLength(3);

    mFontSizeCombo->lineEdit()->setStyleSheet(
                "QLineEdit {"
                    "background: rgb(200, 200, 200);"
                "} "
                "QLineEdit:focus {"
                    "background: rgb(255, 255, 255);"
                "}");
    MainWindow::sGetInstance()->installNumericFilter(mFontSizeCombo);
    mFontSizeCombo->setValidator(new QIntValidator(1, 999, mFontSizeCombo));

    mFontFamilyCombo->addItems(mFontDatabase.families());
    connect(mFontFamilyCombo, &QComboBox::currentTextChanged,
            this, qOverload<const QString &>(
                &FontsWidget::updateStylesFromCurrentFamilyAndEmit));

    connect(mFontStyleCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);

    connect(mFontSizeCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::emitSizeChanged);

    mMainLayout = new QHBoxLayout(this);
    mMainLayout->setSpacing(MIN_WIDGET_DIM);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    setLayout(mMainLayout);
    mMainLayout->addWidget(mFontFamilyCombo);
    mMainLayout->addWidget(mFontStyleCombo);
    mMainLayout->addWidget(mFontSizeCombo);

    const QString iconsDir = eSettings::sIconsDir() + "/toolbarButtons";

    mAlignLeft = new ActionButton(iconsDir + "/alignLeft.png",
                                  "Align Text Left", this);
    connect(mAlignLeft, &ActionButton::pressed,
            this, [this]() { emit textAlignmentChanged(Qt::AlignLeft); });

    mAlignCenter = new ActionButton(iconsDir + "/alignCenter.png",
                                    "Align Text Center", this);
    connect(mAlignCenter, &ActionButton::pressed,
            this, [this]() { emit textAlignmentChanged(Qt::AlignCenter); });

    mAlignRight = new ActionButton(iconsDir + "/alignRight.png",
                                  "Align Text Right", this);
    connect(mAlignRight, &ActionButton::pressed,
            this, [this]() { emit textAlignmentChanged(Qt::AlignRight); });


    mAlignTop = new ActionButton(iconsDir + "/alignTop.png",
                                 "Align Text Top", this);
    connect(mAlignTop, &ActionButton::pressed,
            this, [this]() { emit textVAlignmentChanged(Qt::AlignTop); });

    mAlignVCenter = new ActionButton(iconsDir + "/alignVCenter.png",
                                     "Align Text Center", this);
    connect(mAlignVCenter, &ActionButton::pressed,
            this, [this]() { emit textVAlignmentChanged(Qt::AlignCenter); });

    mAlignBottom = new ActionButton(iconsDir + "/alignBottom.png",
                                    "Align Text Bottom", this);
    connect(mAlignBottom, &ActionButton::pressed,
            this, [this]() { emit textVAlignmentChanged(Qt::AlignBottom); });

    const auto buttonsLayout = new QHBoxLayout;
    buttonsLayout->setSpacing(MIN_WIDGET_DIM/5);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    buttonsLayout->addWidget(mAlignLeft);
    buttonsLayout->addWidget(mAlignCenter);
    buttonsLayout->addWidget(mAlignRight);
    buttonsLayout->addSpacing(MIN_WIDGET_DIM);
    buttonsLayout->addWidget(mAlignTop);
    buttonsLayout->addWidget(mAlignVCenter);
    buttonsLayout->addWidget(mAlignBottom);

    mMainLayout->addLayout(buttonsLayout);

    updateStylesFromCurrentFamilyAndEmit();
}

void FontsWidget::updateStylesFromCurrentFamily(const QString &family) {
    disconnect(mFontStyleCombo, &QComboBox::currentTextChanged,
               this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);

    const QString currentStyle = getCurrentFontStyle();

    mFontStyleCombo->clear();
    QStringList styles = mFontDatabase.styles(family);
    if(styles.isEmpty()) styles << "Regular";
    mFontStyleCombo->addItems(styles);

    if(styles.contains(currentStyle)) {
        mFontStyleCombo->setCurrentText(currentStyle);
    }

    connect(mFontStyleCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);
    updateSizesFromCurrentFamilyAndStyles();
}

void FontsWidget::updateStylesFromCurrentFamilyAndEmit(const QString &family) {
    updateStylesFromCurrentFamily(family);

    emitFamilyAndStyleChanged();
}

void FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit() {
    updateSizesFromCurrentFamilyAndStyles();
    emitFamilyAndStyleChanged();
}

void FontsWidget::updateSizesFromCurrentFamilyAndStyles() {
    disconnect(mFontSizeCombo, &QComboBox::currentTextChanged,
               this, &FontsWidget::emitSizeChanged);
    const QString currentSize = mFontSizeCombo->currentText();

    mFontSizeCombo->clear();
    QList<int> sizes = mFontDatabase.smoothSizes(getCurrentFontFamily(),
                                                       getCurrentFontStyle());
    if(sizes.isEmpty()) sizes = mFontDatabase.standardSizes();
    for(const int size : sizes) {
        mFontSizeCombo->addItem(QString::number(size));
    }

    if(currentSize.isEmpty()) {
        mFontSizeCombo->setCurrentText("72");
    } else {
        mFontSizeCombo->setCurrentText(currentSize);
    }
    connect(mFontSizeCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::emitSizeChanged);
}

void FontsWidget::updateStylesFromCurrentFamilyAndEmit() {
    updateStylesFromCurrentFamilyAndEmit(getCurrentFontFamily());
}

qreal FontsWidget::getCurrentFontSize() const {
    return mFontSizeCombo->currentText().toDouble();
}

QString FontsWidget::getCurrentFontStyle() const {
    return mFontStyleCombo->currentText();
}

QString FontsWidget::getCurrentFontFamily() const {
    return mFontFamilyCombo->currentText();
}

void FontsWidget::setCurrentFontSize(const qreal size) {
    disconnect(mFontSizeCombo, &QComboBox::currentTextChanged,
               this, &FontsWidget::emitSizeChanged);
    mFontSizeCombo->setCurrentText(QString::number(qRound(size)));
    connect(mFontSizeCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::emitSizeChanged);
}

void FontsWidget::setCurrentFontFamily(const QString &family) {
    disconnect(mFontFamilyCombo, &QComboBox::currentTextChanged,
               this, qOverload<const QString &>(
                   &FontsWidget::updateStylesFromCurrentFamilyAndEmit));
    mFontFamilyCombo->setCurrentText(family);
    connect(mFontFamilyCombo, &QComboBox::currentTextChanged,
            this, qOverload<const QString &>(
                &FontsWidget::updateStylesFromCurrentFamilyAndEmit));
    updateStylesFromCurrentFamily(family);
}

void FontsWidget::setCurrentFontStyle(const QString &style) {
    disconnect(mFontStyleCombo, &QComboBox::currentTextChanged,
               this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);
    if(style.isEmpty()) {
        mFontStyleCombo->setCurrentIndex(0);
    } else {
        mFontStyleCombo->setCurrentText(style);
    }
    connect(mFontStyleCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);
    updateSizesFromCurrentFamilyAndStyles();
}

void FontsWidget::setCurrentSettings(const qreal size,
                                     const QString &family,
                                     const QString &style) {
    setCurrentFontFamily(family);
    setCurrentFontStyle(style);
    setCurrentFontSize(size);
}

void FontsWidget::emitFamilyAndStyleChanged() {
    emit fontFamilyAndStyleChanged(getCurrentFontFamily(),
                                   getCurrentFontStyle());
}

void FontsWidget::emitSizeChanged() {
    emit fontSizeChanged(getCurrentFontSize());
}
