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
    QString getCurrentFontFamily() const;

    void setCurrentFontSize(const qreal size);
    void setCurrentFontFamily(const QString &family);
    void setCurrentSettings(const qreal size, const QString &family);
signals:
    void fontFamilyChanged(const QString& family);
    void fontSizeChanged(qreal size);
    void textAlignmentChanged(Qt::Alignment alignment);
    void textVAlignmentChanged(Qt::Alignment alignment);
private:
    void emitFamilyChanged();
    void emitSizeChanged();

    QHBoxLayout *mMainLayout;

    QComboBox *mFontFamilyCombo;
    QComboBox *mFontSizeCombo;

    ActionButton *mAlignLeft;
    ActionButton *mAlignCenter;
    ActionButton *mAlignRight;

    ActionButton *mAlignTop;
    ActionButton *mAlignVCenter;
    ActionButton *mAlignBottom;

    QFontDatabase mFontDatabase;
};

#endif // FONTSWIDGET_H
