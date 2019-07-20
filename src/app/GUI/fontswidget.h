#ifndef FONTSWIDGET_H
#define FONTSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QFontDatabase>

class FontsWidget : public QWidget {
    Q_OBJECT
public:
    FontsWidget(QWidget *parent = nullptr);

    qreal getCurrentFontSize() const;
    QString getCurrentFontStyle() const;
    QString getCurrentFontFamily() const;

    void setCurrentFontSize(const qreal size);
    void setCurrentFontFamily(const QString &family);
    void setCurrentFontStyle(const QString &style);
    void setCurrentSettings(const qreal size,
                            const QString &family,
                            const QString &style);
signals:
    void fontFamilyAndStyleChanged(QString family, QString style);
    void fontSizeChanged(qreal size);
private slots:
    void updateStylesFromCurrentFamilyAndEmit(const QString &family);
    void updateStylesFromCurrentFamilyAndEmit();
    void updateSizesFromCurrentFamilyAndStylesAndEmit();

    void emitFamilyAndStyleChanged();
    void emitSizeChanged();
private:
    void updateStylesFromCurrentFamily(const QString &family);
    void updateSizesFromCurrentFamilyAndStyles();

    QHBoxLayout *mMainLayout;

    QComboBox *mFontFamilyCombo;
    QComboBox *mFontStyleCombo;
    QComboBox *mFontSizeCombo;

    QFontDatabase mFontDatabase;
};

#endif // FONTSWIDGET_H
