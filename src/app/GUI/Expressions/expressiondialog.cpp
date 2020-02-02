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

#include "expressiondialog.h"

#include <QLabel>
#include <QCheckBox>
#include <QScrollBar>
#include <QStatusBar>
#include <QApplication>

#include "Animators/Expressions/expressionparser.h"
#include "Boxes/boundingbox.h"
#include "Private/document.h"
#include "expressioneditor.h"

ExpressionDialog::ExpressionDialog(QrealAnimator* const target,
                                   QWidget * const parent) :
    QDialog(parent), mTarget(target) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Expression " + target->prp_getName());

    const auto windowLayout = new QVBoxLayout(this);
    setLayout(windowLayout);

    const auto mainLayout = new QVBoxLayout;

    mLine = new ExpressionEditor(target, this);

    mainLayout->addWidget(new QLabel("Expression: "));
    mainLayout->addWidget(mLine);

    mErrorLabel = new QLabel(this);
    mErrorLabel->setStyleSheet("QLabel { color: red; }");
    mainLayout->addWidget(mErrorLabel);

    const auto buttonsLayout = new QHBoxLayout;
    const auto applyButton = new QPushButton("Apply", this);
    const auto okButton = new QPushButton("Ok", this);
    const auto cancelButton = new QPushButton("Cancel", this);
    const auto checkBox = new QCheckBox("Auto Apply", this);
    connect(checkBox, &QCheckBox::stateChanged,
            this, [this](const int state) {
        if(state) {
            connect(mLine, &ExpressionEditor::textChanged,
                    this, &ExpressionDialog::apply);
        } else {
            disconnect(mLine, &ExpressionEditor::textChanged,
                       this, &ExpressionDialog::apply);
        }
    });

    buttonsLayout->addWidget(checkBox);
    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    windowLayout->setContentsMargins(0, 0, 0, 0);
    const auto style = QApplication::style();
    mainLayout->setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                   style->pixelMetric(QStyle::PM_LayoutTopMargin),
                                   style->pixelMetric(QStyle::PM_LayoutRightMargin),
                                   style->pixelMetric(QStyle::PM_LayoutBottomMargin));
    windowLayout->addLayout(mainLayout);
    const auto statusBar = new QStatusBar(this);
    windowLayout->addWidget(statusBar);
    statusBar->showMessage("Use Ctrl + Space for suggestions", 10000);

    connect(applyButton, &QPushButton::released,
            this, &ExpressionDialog::apply);
    connect(okButton, &QPushButton::released,
            this, &ExpressionDialog::apply);
    connect(okButton, &QPushButton::released,
            this, &ExpressionDialog::accept);
    connect(cancelButton, &QPushButton::released,
            this, &ExpressionDialog::reject);
}

void ExpressionDialog::apply() {
    mErrorLabel->clear();
    ExpressionValue::sptr expr;
    const auto text = mLine->toPlainText();
    try {
        expr = ExpressionParser::parse(text, mTarget);
        if(!expr || !expr->isValid()) {
            mTarget->clearExpression();
            RuntimeThrow("Invalid expression.");
        } else {
            mTarget->setExpression(expr);
        }
        Document::sInstance->actionFinished();
    } catch(const std::exception& e) {
        const QString error = e.what();
        const QString lastLine = error.split(QRegExp("\n|\r\n|\r")).last();
        mErrorLabel->setText(lastLine);
    }
}
