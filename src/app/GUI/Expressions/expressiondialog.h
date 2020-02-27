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

#include <Qsci/qsciscintilla.h>

class QrealAnimator;
class ExpressionEditor;
class PropertyBinding;
class QsciLexerJavaScript;

class QsciAPIs;

class ExpressionDialog : public QDialog {
public:
    ExpressionDialog(QrealAnimator* const target,
                     QWidget * const parent = nullptr);

private:
    using PropertyBindingMap = std::map<QString, QSharedPointer<PropertyBinding>>;
    bool getBindings(PropertyBindingMap& bindings);
    void updateBindingsAutocomplete();

    bool apply(const bool action);

    QrealAnimator* const mTarget;

    ExpressionEditor* mBindings;
    QLabel* mBindingsError;

    QsciLexerJavaScript* mDefsLexer;
    QsciScintilla* mDefinitions;
    QsciAPIs* mDefinitionsApi;
    QLabel* mDefinitionsError;

    QsciLexerJavaScript* mScriptLexer;
    QStringList mDefinitionList;
    QStringList mBindingsList;
    QLabel* mScriptLabel;
    QsciScintilla* mScript;
    QsciAPIs* mScriptApi;
    QLabel* mScriptError;
};

#endif // EXPRESSIONDIALOG_H
