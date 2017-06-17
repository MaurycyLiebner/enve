#include "fontswidget.h"
#include <QLineEdit>
#include <QIntValidator>

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

    mFontSizeCombo->setValidator(new QIntValidator(0, 9999, mFontSizeCombo));

    mFontFamilyCombo->addItems(mFontDatabase.families());
    connect(mFontFamilyCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateStylesFromCurrentFamilyAndEmit(QString)));

    connect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateSizesFromCurrentFamilyAndStylesAndEmit()) );

    connect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );

    updateStylesFromCurrentFamilyAndEmit();

    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->addWidget(mFontFamilyCombo);
    mMainLayout->addWidget(mFontStyleCombo);
    mMainLayout->addWidget(mFontSizeCombo);
}

void FontsWidget::updateStylesFromCurrentFamily(const QString &family) {
    disconnect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
               this, SLOT(updateSizesFromCurrentFamilyAndStylesAndEmit()) );

    QString currentStyle = getCurrentFontStyle();

    mFontStyleCombo->clear();
    QStringList styles = mFontDatabase.styles(family);
    mFontStyleCombo->addItems(styles );

    if(styles.contains(currentStyle) ) {
        mFontStyleCombo->setCurrentText(currentStyle);
    }

    connect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateSizesFromCurrentFamilyAndStylesAndEmit()) );
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
    disconnect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );
    QString currentSize = mFontSizeCombo->currentText();

    mFontSizeCombo->clear();
    QList<int> sizes = mFontDatabase.smoothSizes(getCurrentFontFamily(),
                                                 getCurrentFontStyle() );
    Q_FOREACH(int size, sizes) {
        mFontSizeCombo->addItem(QString::number(size) );
    }

    mFontSizeCombo->setCurrentText(currentSize);
    connect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );
}

void FontsWidget::updateStylesFromCurrentFamilyAndEmit() {
    updateStylesFromCurrentFamilyAndEmit(getCurrentFontFamily() );
}

qreal FontsWidget::getCurrentFontSize() {
    return mFontSizeCombo->currentText().toDouble();
}

QString FontsWidget::getCurrentFontStyle() {
    return mFontStyleCombo->currentText();
}

QString FontsWidget::getCurrentFontFamily() {
    return mFontFamilyCombo->currentText();
}

void FontsWidget::setCurrentFontSize(const qreal &size) {
    disconnect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );
    mFontSizeCombo->setCurrentText(QString::number((int)size));
    connect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );
}

void FontsWidget::setCurrentFontFamily(const QString &family) {
    disconnect(mFontFamilyCombo, SIGNAL(currentTextChanged(QString)),
               this, SLOT(updateStylesFromCurrentFamilyAndEmit(QString)));
    mFontFamilyCombo->setCurrentText(family);
    connect(mFontFamilyCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateStylesFromCurrentFamilyAndEmit(QString)));
    updateStylesFromCurrentFamily(family);
}

void FontsWidget::setCurrentFontStyle(const QString &style) {
    disconnect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
               this, SLOT(updateSizesFromCurrentFamilyAndStylesAndEmit()) );
    mFontStyleCombo->setCurrentText(style);
    connect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateSizesFromCurrentFamilyAndStylesAndEmit()) );
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
