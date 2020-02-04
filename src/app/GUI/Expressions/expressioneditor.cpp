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

#include "expressioneditor.h"

#include <QScrollBar>

#include "expressionhighlighter.h"

ExpressionEditor::ExpressionEditor(QrealAnimator * const target,
                                   QWidget * const parent) :
    ExpressionEditor(target, target->expressionText(), parent) {}

ExpressionEditor::ExpressionEditor(QrealAnimator * const target,
                                   const QString &text,
                                   QWidget * const parent) :
    QTextEdit(parent) {
    setMinimumWidth(400);
    const auto doc = document();
    setAcceptRichText(false);
    mHighlighter = new ExpressionHighlighter(target, this, doc);
    connect(this, &QTextEdit::cursorPositionChanged,
            this, [this]() {
        const auto cursor = textCursor();
        mHighlighter->setCursor(cursor);
    });
    mCompleter = new QCompleter(this);
    mCompleter->setWidget(this);
    mCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mCompleter->setCaseSensitivity(Qt::CaseSensitive);
    connect(mCompleter,
            qOverload<const QString&>(&QCompleter::activated),
            this, &ExpressionEditor::insertCompletion);
    mVariableDefinition = QRegularExpression("\\$[A-Za-z_][A-Za-z0-9_]* *=");
    connect(doc, &QTextDocument::contentsChange,
            this, &ExpressionEditor::updateVariables);
    setText(text);
}

void ExpressionEditor::setCompleterList(const QStringList &values) {
    mCompleter->setModel(new QStringListModel(values, mCompleter));
}

void ExpressionEditor::keyPressEvent(QKeyEvent *e) {
    const auto key = e->key();
    const bool wasComplete = mCompleter->popup()->isVisible();
    const auto mods = e->modifiers();
    const bool ctrlPressed = mods.testFlag(Qt::ControlModifier);
    const bool spacePressed = key == Qt::Key_Space;
    const bool isShortcut = ctrlPressed && spacePressed;
    const auto popup = mCompleter->popup();
    const bool popupVisible = popup->isVisible();
    if((key == Qt::Key_Return || key == Qt::Key_Tab) && popupVisible) {
        return QWidget::keyPressEvent(e);
    } else if(isShortcut) {
        showCompleter();
    } else QTextEdit::keyPressEvent(e);
    const bool input = e->text().contains(QRegExp("[A-Za-z0-9_ \\.\\$]"));
    const bool deletion = key == Qt::Key_Delete ||
            key == Qt::Key_Backspace;
    const bool arrows = key == Qt::Key_Right ||
            key == Qt::Key_Left;
    const bool complete = input || deletion || (arrows && wasComplete);
    if(complete) {
        QTimer::singleShot(1, this, [this]() { showCompleter(); });
    }
}

void ExpressionEditor::updateVariables(const int from,
                                       const int charsRemoved,
                                       const int charsAdded) {
    const auto doc = document();
    QTextBlock block = doc->findBlock(from);
    if(!block.isValid()) return;

    QTextBlock lastBlock = doc->findBlock(from + charsAdded + (charsRemoved > 0 ? 1 : 0));
    if(!lastBlock.isValid()) lastBlock = doc->lastBlock();
    const int endPosition = lastBlock.position() + lastBlock.length();

    bool changed = false;
    while(block.isValid() && (block.position() < endPosition)) {
        const int blockId = block.blockNumber();
        auto& variables = mVariables[blockId];
        const auto oldVariables = variables;
        variables.clear();
        {
            auto matchIterator = mVariableDefinition.globalMatch(block.text());
            while(matchIterator.hasNext()) {
                const auto match = matchIterator.next();
                auto def = match.captured();
                variables << def.remove('$').remove('=').trimmed();
            }
        }
        for(const auto& var : variables) {
            if(oldVariables.contains(var)) continue;
            mHighlighter->addVariable(var);
            changed = true;
        }
        for(const auto& var : oldVariables) {
            if(variables.contains(var)) continue;
            mHighlighter->removeVariable(var);
            changed = true;
        }
        block = block.next();
    }
    if(changed) mHighlighter->updateVariablesRule();
}

void ExpressionEditor::showCompleter() {
    const auto popup = mCompleter->popup();
    const auto underCursor = textUnderCursor();
    mCompleter->setCompletionPrefix(underCursor);
    QRect cr = cursorRect();
    const auto scrollBar = popup->verticalScrollBar();
    const int scrollBarWidth = scrollBar->sizeHint().width();
    cr.setWidth(popup->sizeHintForColumn(0) + scrollBarWidth);
    mCompleter->complete(cr);
}

void ExpressionEditor::insertCompletion(const QString &completion) {
    QTextCursor tc = textCursor();
    const int prefixLen = mCompleter->completionPrefix().length();
    const int extra = completion.length() - prefixLen;
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    if(completion.right(1) == ')') tc.movePosition(QTextCursor::Left);
    setTextCursor(tc);
}

QString ExpressionEditor::textUnderCursor() const {
    QTextCursor tc = textCursor();
    QChar prevCharacter;
    {
        auto checkSpace = tc;
        checkSpace.movePosition(QTextCursor::Left,
                                QTextCursor::KeepAnchor);
        const auto selected = checkSpace.selectedText();
        if(selected.isEmpty()) prevCharacter = ' ';
        else prevCharacter = selected.front();
    }
    if(prevCharacter == ' ') return "";
    else if(prevCharacter == '$') return "$";
    else if(!prevCharacter.isLetter() && !prevCharacter.isDigit()) return "";

    tc.movePosition(QTextCursor::Left);
    tc.select(QTextCursor::WordUnderCursor);

    QString result = tc.selectedText();
    {
        auto checkVar = tc;
        checkVar.movePosition(QTextCursor::Left,
                              QTextCursor::MoveAnchor);
        checkVar.movePosition(QTextCursor::Left,
                              QTextCursor::KeepAnchor);
        const auto selected = checkVar.selectedText();
        if(selected == "$") result.prepend('$');
    }
    return result;
}

