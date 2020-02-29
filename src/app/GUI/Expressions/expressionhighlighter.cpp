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

    mObjectsExpression = QRegularExpression(
                "\\b([A-Za-z_]([A-Za-z0-9_ ]*[A-Za-z0-9_])*\\.?)+");
    QTextCharFormat objectFormat;
    objectFormat.setForeground(QColor(255, 128, 128));
    objectFormat.setBackground(QColor(45, 45, 45));
    rule.pattern = mObjectsExpression;
    rule.format = objectFormat;
    mHighlightingRules.append(rule);

    const QStringList specs = {
        QStringLiteral("$value"),
        QStringLiteral("$frame")
    };

    QTextCharFormat specialFormat;
    specialFormat.setForeground(QColor(185, 255, 155));
    rule.format = specialFormat;
    for(const QString &spec : specs) {
        rule.pattern = QRegularExpression(QString("\\%1\\b").arg(spec));
        mHighlightingRules.append(rule);
        mBaseComplete << spec;
    }

    QTextCharFormat commentFormat;
    commentFormat.setForeground(QColor(125, 125, 125));
    rule.pattern = QRegularExpression("\\/\\/.*");;
    rule.format = objectFormat;
    mHighlightingRules.append(rule);

    mBaseVarsComplete = mBaseComplete;
}

void ExpressionHighlighter::setCursor(const QTextCursor& cursor) {
    mCursorPos = cursor.positionInBlock();
    const auto oldBlock = mOldCursor.block();
    if(oldBlock.isValid()) rehighlightBlock(oldBlock);
    rehighlightBlock(cursor.block());
    mOldCursor = cursor;
}

void ExpressionHighlighter::addVariable(const QString &name) {
    mVariables << name;
}

void ExpressionHighlighter::removeVariable(const QString &name){
    mVariables.removeAll(name);
}

void ExpressionHighlighter::highlightBlock(const QString &text) {
    for(const auto &rule : qAsConst(mHighlightingRules)) {
        auto matchIterator = rule.pattern.globalMatch(text);
        while(matchIterator.hasNext()) {
            const auto match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
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
                const auto qra = enve_cast<QrealAnimator*>(obj);
                error = obj == mTarget ||
                        (!qra && !enve_cast<ComplexAnimator*>(obj));
                if(!error && qra) {
                    error = qra->prp_dependsOn(mTarget);
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
        QStringList tmp = mBaseVarsComplete;
        tmp.append(completions);
        completions = tmp;
    }
    mEditor->setCompleterList(completions);
}
