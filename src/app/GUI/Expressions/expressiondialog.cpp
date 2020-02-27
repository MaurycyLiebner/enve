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

#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qsciapis.h>

#include "Expressions/expression.h"
#include "Boxes/boundingbox.h"
#include "Private/document.h"
#include "expressioneditor.h"

class eEditor : public QsciScintilla {

};

ExpressionDialog::ExpressionDialog(QrealAnimator* const target,
                                   QWidget * const parent) :
    QDialog(parent), mTarget(target) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Expression " + target->prp_getName());

    const auto windowLayout = new QVBoxLayout(this);
    setLayout(windowLayout);

    const auto mainLayout = new QVBoxLayout;

    mBindings = new ExpressionEditor(target, this);
    connect(mBindings, &ExpressionEditor::focusLost,
            this, &ExpressionDialog::updateBindingsAutocomplete);

    mainLayout->addWidget(new QLabel("Bindings:"));
    mainLayout->addWidget(mBindings);

    mBindingsError = new QLabel(this);
    mBindingsError->setObjectName("errorLabel");
    mainLayout->addWidget(mBindingsError);

    mainLayout->addWidget(new QLabel("Definitions:"));
    mDefinitions = new QsciScintilla;
    mDefsLexer = new QsciLexerJavaScript(mDefinitions);
    mDefinitionsApi = new QsciAPIs(mDefsLexer);

    mDefinitionsApi->add("function");
    mDefinitionsApi->add("var");
    mDefinitionsApi->add("return");

    mDefinitionsApi->prepare();
    mDefinitions->setLexer(mDefsLexer);
    mDefinitions->setAutoCompletionSource(QsciScintilla::AcsAll);
    mDefinitions->setText(target->getExpressionDefinitionsString());
    connect(mDefinitions, &QsciScintilla::textChanged,
            mDefinitions, &QsciScintilla::autoCompleteFromAll);

    mainLayout->addWidget(mDefinitions);

    mDefinitionsError = new QLabel(this);
    mDefinitionsError->setObjectName("errorLabel");
    mainLayout->addWidget(mDefinitionsError);

    mScriptLabel = new QLabel("Script (  ) :");
    mainLayout->addWidget(mScriptLabel);
    mScript = new QsciScintilla;
    mScriptLexer = new QsciLexerJavaScript(mScript);

    mScriptApi = new QsciAPIs(mScriptLexer);

    mScriptApi->add("function");
    mScriptApi->add("var");

    mScriptApi->prepare();

    mScript->setLexer(mScriptLexer);
    mScript->setAutoCompletionSource(QsciScintilla::AcsAll);
    mScript->setText(target->getExpressionScriptString());
    connect(mScript, &QsciScintilla::textChanged,
            mScript, &QsciScintilla::autoCompleteFromAll);
    mainLayout->addWidget(mScript);

    mScriptError = new QLabel(this);
    mScriptError->setObjectName("errorLabel");
    mainLayout->addWidget(mScriptError);

    const auto buttonsLayout = new QHBoxLayout;
    const auto applyButton = new QPushButton("Apply", this);
    const auto okButton = new QPushButton("Ok", this);
    const auto cancelButton = new QPushButton("Cancel", this);
    const auto checkBox = new QCheckBox("Auto Apply", this);
    connect(checkBox, &QCheckBox::stateChanged,
            this, [this](const int state) {
        if(state) {
            connect(mBindings, &ExpressionEditor::textChanged,
                    this, [this]() { apply(false); });
            connect(mDefinitions, &QsciScintilla::textChanged,
                    this, [this]() { apply(false); });
            connect(mScript, &QsciScintilla::textChanged,
                    this, [this]() { apply(false); });
        } else {
            disconnect(mBindings, &ExpressionEditor::textChanged,
                       this, nullptr);
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

    connect(applyButton, &QPushButton::released,
            this, [this]() { apply(true); });
    connect(okButton, &QPushButton::released,
            this, [this]() {
        const bool valid = apply(true);
        if(valid) accept();
    });
    connect(cancelButton, &QPushButton::released,
            this, &ExpressionDialog::reject);

    setTabOrder(mBindings, mDefinitions);
    setTabOrder(mDefinitions, mScript);
    setTabOrder(mScript, mBindings);

    connect(mDefinitions, &QsciScintilla::SCN_FOCUSIN,
            this, [this]() {
        for(const auto& binding : mBindingsList) {
            mDefinitionsApi->remove(binding);
        }
        mDefinitionsApi->prepare();
        mDefsLexer->setAPIs(mDefinitionsApi);
    });
    connect(mScript, &QsciScintilla::SCN_FOCUSIN,
            this, [this]() {
        for(const auto& binding : mBindingsList) {
            mDefinitionsApi->add(binding);
        }
        mDefinitionsApi->prepare();
        mScriptLexer->setAPIs(mDefinitionsApi);
    });
}

bool ExpressionDialog::getBindings(PropertyBindingMap& bindings) {
    mBindingsError->clear();
    const auto bindingsStr = mBindings->toPlainText();
    try {
        bindings = PropertyBindingParser::parseBindings(
                       bindingsStr, nullptr, mTarget);
        return true;
    } catch(const std::exception& e) {
        mBindingsError->setText(e.what());
        return false;
    }
}

void ExpressionDialog::updateBindingsAutocomplete() {
    for(const auto& binding : mBindingsList) {
        mScriptApi->remove(binding);
    }
    mBindingsList.clear();
    PropertyBindingMap bindings;
    if(getBindings(bindings)) {
        for(const auto& binding : bindings) {
            mBindingsList << binding.first;
            mScriptApi->add(binding.first);
        }
    }
    mScriptLabel->setText("Script ( <b>" + mBindingsList.join("</b>, <b>") + "</b> ) :");
    mScriptApi->prepare();
}

bool ExpressionDialog::apply(const bool action) {
    mDefinitionsError->clear();
    mScriptError->clear();

    const auto definitionsStr = mDefinitions->text();
    const auto scriptStr = mScript->text();

    PropertyBindingMap bindings;
    if(!getBindings(bindings)) return false;

    auto engine = std::make_unique<QJSEngine>();
    try {
        Expression::sAddDefinitionsTo(definitionsStr, *engine);
    } catch(const std::exception& e) {
        mDefinitionsError->setText(e.what());
        return false;
    }

    QJSValue eEvaluate;
    try {
        Expression::sAddScriptTo(scriptStr, bindings, *engine, eEvaluate);
        Q_ASSERT(eEvaluate.isCallable());
    } catch(const std::exception& e) {
        mScriptError->setText(e.what());
        return false;
    }

    try {
        auto expr = Expression::sCreate(definitionsStr,
                                        scriptStr, std::move(bindings),
                                        std::move(engine),
                                        std::move(eEvaluate));
        if(expr && !expr->isValid()) expr = nullptr;
        if(action) {
            mTarget->setExpressionAction(expr);
        } else {
            mTarget->setExpression(expr);
        }
    } catch(const std::exception& e) {
        return false;
    }

    Document::sInstance->actionFinished();
    return true;
}
