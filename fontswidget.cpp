#include "fontswidget.h"

FontsWidget::FontsWidget(QWidget *parent) : QWidget(parent)
{
    mFontStyleCombo = new QComboBox(this);
    mFontFamilyCombo = new QComboBox(this);
    mFontSizeCombo = new QComboBox(this);
    mFontSizeCombo->setEditable(true);

    mFontFamilyCombo->addItems(mFontDatabase.families());
    connect(mFontFamilyCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateStylesFromCurrentFamily(QString)));

    connect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(updateSizesFromCurrentFamilyAndStyles()) );

    connect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitFamilyAndStyleChanged()));

    connect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );

    updateStylesFromCurrentFamily();

    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->addWidget(mFontFamilyCombo);
    mMainLayout->addWidget(mFontStyleCombo);
    mMainLayout->addWidget(mFontSizeCombo);
}

void FontsWidget::updateStylesFromCurrentFamily(QString family)
{
    disconnect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitFamilyAndStyleChanged()));

    QString currentStyle = getCurrentFontStyle();

    mFontStyleCombo->clear();
    QStringList styles = mFontDatabase.styles(family);
    mFontStyleCombo->addItems(styles );

    if(styles.contains(currentStyle) ) {
        mFontStyleCombo->setCurrentText(currentStyle);
    }

    emitFamilyAndStyleChanged();

    connect(mFontStyleCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitFamilyAndStyleChanged()));
}

void FontsWidget::updateSizesFromCurrentFamilyAndStyles() {
    disconnect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );
    QString currentSize = mFontSizeCombo->currentText();

    mFontSizeCombo->clear();
    QList<int> sizes = mFontDatabase.smoothSizes(getCurrentFontFamily(),
                                                 getCurrentFontStyle() );
    foreach(int size, sizes) {
        mFontSizeCombo->addItem(QString::number(size) );
    }

    mFontSizeCombo->setCurrentText(currentSize);
    connect(mFontSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(emitSizeChanged()) );
}

void FontsWidget::updateStylesFromCurrentFamily()
{
    updateStylesFromCurrentFamily(getCurrentFontFamily() );
}

qreal FontsWidget::getCurrentFontSize()
{
    return mFontSizeCombo->currentText().toDouble();
}

QString FontsWidget::getCurrentFontStyle()
{
    return mFontStyleCombo->currentText();
}

QString FontsWidget::getCurrentFontFamily()
{
    return mFontFamilyCombo->currentText();
}

void FontsWidget::emitFamilyAndStyleChanged()
{
    emit fontFamilyAndStyleChanged(getCurrentFontFamily(),
                                   getCurrentFontStyle());
}

void FontsWidget::emitSizeChanged()
{
    emit fontSizeChanged(getCurrentFontSize());
}
