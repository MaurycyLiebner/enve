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

#ifndef FONTSWIDGET_H
#define FONTSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QFontDatabase>
#include "actionbutton.h"

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
    void textAlignmentChanged(Qt::Alignment alignment);
private:
    void updateStylesFromCurrentFamilyAndEmit(const QString &family);
    void updateStylesFromCurrentFamilyAndEmit();
    void updateSizesFromCurrentFamilyAndStylesAndEmit();

    void emitFamilyAndStyleChanged();
    void emitSizeChanged();

    void updateStylesFromCurrentFamily(const QString &family);
    void updateSizesFromCurrentFamilyAndStyles();

    QHBoxLayout *mMainLayout;

    QComboBox *mFontFamilyCombo;
    QComboBox *mFontStyleCombo;
    QComboBox *mFontSizeCombo;

    ActionButton *mAlignLeft;
    ActionButton *mAlignCenter;
    ActionButton *mAlignRight;

    QFontDatabase mFontDatabase;
};

#endif // FONTSWIDGET_H
