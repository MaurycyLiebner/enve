#include "fontswidget.h"
#include <QLineEdit>
#include <QIntValidator>
#include <QLabel>

FontsWidget::FontsWidget(QWidget *parent) : QWidget(parent) {
    mFontStyleCombo = new QComboBox(this);
    mFontFamilyCombo = new QComboBox(this);
    mFontSizeCombo = new QComboBox(this);
    mFontSizeCombo->setEditable(true);
    mFontSizeCombo->setMinimumContentsLength(3);
    mFontSizeCombo->lineEdit()->setStyleSheet(
                "QLineEdit {"
                    "background: rgb(200, 200, 200);"
                "} "
                "QLineEdit:focus {"
                    "background: rgb(255, 255, 255);"
                "}");
    mFontSizeCombo->setProperty("forceHandleEvent", QVariant(true));
    mFontSizeCombo->setValidator(new QIntValidator(0, 999, mFontSizeCombo));

    mFontFamilyCombo->addItems(mFontDatabase.families());
    connect(mFontFamilyCombo, &QComboBox::currentTextChanged,
            this, qOverload<const QString &>(
                &FontsWidget::updateStylesFromCurrentFamilyAndEmit));

    connect(mFontStyleCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);

    connect(mFontSizeCombo, &QComboBox::currentTextChanged,
            this, &FontsWidget::emitSizeChanged);

    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->addWidget(mFontFamilyCombo);
    mMainLayout->addWidget(mFontStyleCombo);
    mMainLayout->addWidget(mFontSizeCombo);

    updateStylesFromCurrentFamilyAndEmit();
}

void FontsWidget::updateStylesFromCurrentFamily(const QString &family) {
    disconnect(mFontStyleCombo, &QComboBox::currentTextChanged,
               this, &FontsWidget::updateSizesFromCurrentFamilyAndStylesAndEmit);

    const QString currentStyle = getCurrentFontStyle();

    mFontStyleCombo->clear();
    const QStringList styles = mFontDatabase.styles(family);
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
    const QList<int> sizes = mFontDatabase.smoothSizes(getCurrentFontFamily(),
                                                       getCurrentFontStyle());
    for(const int &size : sizes) {
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

void FontsWidget::setCurrentFontSize(const qreal &size) {
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

void FontsWidget::setCurrentSettings(const qreal &size,
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
