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
    doc->setMaximumBlockCount(1);
    setAcceptRichText(false);
    const auto highligter = new ExpressionHighlighter(target, this, doc);
    setFixedHeight(13*MIN_WIDGET_DIM/10);
    connect(this, &QTextEdit::cursorPositionChanged,
            this, [this, highligter]() {
        const auto cursor = textCursor();
        highligter->setCursorPos(cursor.positionInBlock());
    });
    mCompleter = new QCompleter(this);
    mCompleter->setWidget(this);
    mCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mCompleter->setCaseSensitivity(Qt::CaseSensitive);
    QObject::connect(mCompleter,
                     qOverload<const QString&>(&QCompleter::activated),
                     this, &ExpressionEditor::insertCompletion);
    setText(text); // force autocomplete update
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
    if(key == Qt::Key_Return ||
            key == Qt::Key_Tab) {
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
    {
        auto checkSpace = tc;
        checkSpace.movePosition(QTextCursor::Left,
                                QTextCursor::KeepAnchor);
        if(checkSpace.selectedText() == " ") {
            return "";
        }
    }
    tc.movePosition(QTextCursor::Left);
    tc.select(QTextCursor::WordUnderCursor);
    QString result = tc.selectedText();
    if(result == ".") {
        tc.movePosition(QTextCursor::Right);
        tc.select(QTextCursor::WordUnderCursor);
        result = tc.selectedText();
    }
    return result;
}

