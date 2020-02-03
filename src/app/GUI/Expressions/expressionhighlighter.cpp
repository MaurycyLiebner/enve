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

#include "expressionhighlighter.h"

#include "Animators/complexanimator.h"
#include "expressioneditor.h"

ExpressionHighlighter::ExpressionHighlighter(
        QrealAnimator * const target,
        ExpressionEditor * const editor,
        QTextDocument *parent) :
    QSyntaxHighlighter(parent),
    mTarget(target),
    mSearchCtxt(target->getParent()),
    mEditor(editor) {
    HighlightingRule rule;

    const QStringList funcs = {
        QStringLiteral("sin"),
        QStringLiteral("cos"),
        QStringLiteral("tan"),
        QStringLiteral("asin"),
        QStringLiteral("acos"),
        QStringLiteral("atan"),
        QStringLiteral("exp"),
        QStringLiteral("abs"),
        QStringLiteral("sqrt"),
        QStringLiteral("rand")
    };

    QTextCharFormat funcFormat;
    funcFormat.setForeground(Qt::white);
    funcFormat.setFontWeight(QFont::Bold);
    rule.format = funcFormat;
    for (const QString &func : funcs) {
        rule.pattern = QRegularExpression(QString("(\\b|(?<=[0-9]))%1\\(").arg(func));
        mFuncRules.append(rule);
        mBaseComplete << func + "()";
    }

    QTextCharFormat operatorFormat;
    operatorFormat.setForeground(QColor(155, 255, 255));
    rule.pattern = QRegularExpression(QStringLiteral("[%/\\-\\^\\+\\*]"));
    rule.format = operatorFormat;
    mHighlightingRules.append(rule);

    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor(255, 225, 155));
    rule.pattern = QRegularExpression(
                QStringLiteral("\\b("
                               "([0-9]+[\\.]?[0-9]*)|"
                               "([0-9]*[\\.]?[0-9]+)"
                               ")"));
    rule.format = numberFormat;
    mHighlightingRules.append(rule);

    mErrorFormat.setForeground(Qt::red);

    mBracketsHighlightFormat.setBackground(QColor(0, 125, 200));

    const auto funcExclude = funcs.join(QStringLiteral("\\(|")) + "\\(";
    mObjectsExpression = QRegularExpression(
                "(\\b|(?<=[0-9]))"
                "(?!(?<=\\$))"
                "(?!" + funcExclude + ")"
                "([A-Za-z_]([A-Za-z0-9_ ]*[A-Za-z0-9_])*\\.?)+");
    QTextCharFormat objectFormat;
    objectFormat.setForeground(QColor(255, 128, 128));
    objectFormat.setBackground(QColor(45, 45, 45));
    rule.pattern = mObjectsExpression;
    rule.format = objectFormat;
    mHighlightingRules.append(rule);

    const QStringList specs = {
        QStringLiteral("value"),
        QStringLiteral("frame")
    };

    QTextCharFormat specialFormat;
    specialFormat.setForeground(QColor(185, 255, 155));
    rule.format = specialFormat;
    for (const QString &spec : specs) {
        rule.pattern = QRegularExpression(QString("\\$%1\\b").arg(spec));
        mHighlightingRules.append(rule);
        mBaseComplete << "$" + spec;
    }
}

void ExpressionHighlighter::setCursorPos(const int pos) {
    mCursorPos = pos;
    setDocument(document());
}

void ExpressionHighlighter::highlightBlock(const QString &text) {
    QList<int> openBrackets;
    for(int i = 0; i < text.count(); i++) {
        const auto& character = text.at(i);
        if(character == '(') openBrackets << i;
        else if(character == ')') {
            if(openBrackets.isEmpty()) setFormat(i, 1, mErrorFormat);
            else {
                const int openPos = openBrackets.last();
                if(i + 1 == mCursorPos || openPos == mCursorPos) {
                    setFormat(i, 1, mBracketsHighlightFormat);
                    setFormat(openPos, 1, mBracketsHighlightFormat);
                }
                openBrackets.removeLast();
            }
        }
    }
    for(int i : openBrackets) setFormat(i, 1, mErrorFormat);

    for(const auto &rule : qAsConst(mHighlightingRules)) {
        auto matchIterator = rule.pattern.globalMatch(text);
        while(matchIterator.hasNext()) {
            const auto match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    for(const auto &rule : qAsConst(mFuncRules)) {
        auto matchIterator = rule.pattern.globalMatch(text);
        while(matchIterator.hasNext()) {
            const auto match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength() - 1, rule.format);
        }
    }

    bool objCompletSetup = false;
    bool addFuncsComplete = true;
    QStringList completions;
    auto matchIterator = mObjectsExpression.globalMatch(text);
    while(matchIterator.hasNext()) {
        const auto match = matchIterator.next();
        const auto path = match.capturedTexts().first();
        const auto objs = path.split('.');
        for(int i = objs.count() - 1; i >= 0; i--) {
            const auto subPath = objs.mid(0, objs.count() - i);
            int min = match.capturedStart();
            for(int j = 0; j < subPath.count() - 1; j++) {
                const auto& jObj = subPath.at(j);
                min += jObj.count() + 1;
            }
            const auto& lastObjPath = subPath.last();
            const int max = min + lastObjPath.count();
            const bool autocomplete = mCursorPos >= min && mCursorPos <= max;
            const auto obj = mSearchCtxt->ca_findPropertyWithPathRec(
                        0, subPath, autocomplete ? &completions : nullptr);
            if(autocomplete) {
                if(objs.count() > 1) addFuncsComplete = false;
                completions.removeDuplicates();
                objCompletSetup = true;
            }

            bool error = !obj;
            if(!error) {
                const bool isQra = obj->SWT_isQrealAnimator();
                error = obj == mTarget ||
                        (!isQra && !obj->SWT_isComplexAnimator());
                if(!error && isQra) {
                    const auto qra = static_cast<QrealAnimator*>(obj);
                    error = qra->expressionDependsOn(mTarget);
                }
            }
            if(error) {
                const int len = match.capturedEnd() - min;
                setFormat(min, len, mErrorFormat);
                break;
            }
        }
    }
    if(!objCompletSetup) {
        mSearchCtxt->ca_findPropertyWithPathRec(
                    0, QStringList() << "", &completions);
    }
    if(addFuncsComplete) {
        QStringList tmp = mBaseComplete;
        tmp.append(completions);
        completions = tmp;
    }
    mEditor->setCompleterList(completions);
}
