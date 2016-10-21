#ifndef FONTSWIDGET_H
#define FONTSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QFontDatabase>

class FontsWidget : public QWidget
{
    Q_OBJECT
public:
    FontsWidget(QWidget *parent = NULL);

    qreal getCurrentFontSize();
    QString getCurrentFontStyle();
    QString getCurrentFontFamily();

signals:
    void fontFamilyAndStyleChanged(QString family, QString style);
    void fontSizeChanged(qreal size);
private slots:
    void updateStylesFromCurrentFamily(QString family);
    void updateStylesFromCurrentFamily();
    void updateSizesFromCurrentFamilyAndStyles();

    void emitFamilyAndStyleChanged();
    void emitSizeChanged();
private:
    QHBoxLayout *mMainLayout;

    QComboBox *mFontFamilyCombo;
    QComboBox *mFontStyleCombo;
    QComboBox *mFontSizeCombo;

    QFontDatabase mFontDatabase;
};

#endif // FONTSWIDGET_H
