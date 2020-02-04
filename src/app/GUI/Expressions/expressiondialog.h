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

#ifndef EXPRESSIONDIALOG_H
#define EXPRESSIONDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class QrealAnimator;
class ExpressionEditor;

class ExpressionDialog : public QDialog {
public:
    ExpressionDialog(QrealAnimator* const target,
                     QWidget * const parent = nullptr);

private:
    bool apply(const bool action);

    QrealAnimator* const mTarget;

    ExpressionEditor* mLine;
    QLabel* mErrorLabel;
};

#endif // EXPRESSIONDIALOG_H
