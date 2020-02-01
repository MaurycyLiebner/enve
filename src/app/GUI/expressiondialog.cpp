#include "expressiondialog.h"

#include <QLabel>
#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QCheckBox>
#include <QStringListModel>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>

#include "Animators/qrealanimator.h"
#include "Animators/Expressions/expressionparser.h"
#include "GUI/global.h"

#include "Private/document.h"

class Highlighter : public QSyntaxHighlighter {
public:
    Highlighter(QrealAnimator* const target,
                ExpressionEditor* const editor,
                QTextDocument *parent) :
        QSyntaxHighlighter(parent),
        mTarget(target),
        mSearchCtxt(target->getParent()),
        mEditor(editor) {
        HighlightingRule rule;

        QTextCharFormat objectFormat;
        objectFormat.setForeground(QColor(255, 128, 128));
        rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*\\b"));
        rule.format = objectFormat;
        mHighlightingRules.append(rule);

        QTextCharFormat funcFormat;
        funcFormat.setFontWeight(QFont::Bold);
        const QString funcs[] = {
            QStringLiteral("\\bsin\\(\\b"),
            QStringLiteral("\\bcos\\(\\b"),
            QStringLiteral("\\btan\\(\\b"),
            QStringLiteral("\\basin\\(\\b"),
            QStringLiteral("\\bacos\\(\\b"),
            QStringLiteral("\\batan\\(\\b"),
            QStringLiteral("\\bexp\\(\\b")
        };

        for (const QString &pattern : funcs) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = funcFormat;
            mFuncRules.append(rule);
        }

        QTextCharFormat operatorFormat;
        operatorFormat.setForeground(QColor(155, 255, 255));
        rule.pattern = QRegularExpression(QStringLiteral("[%/\\-\\^\\+\\*]"));
        rule.format = operatorFormat;
        mHighlightingRules.append(rule);

        QTextCharFormat numberFormat;
        numberFormat.setForeground(QColor(255, 225, 155));
        rule.pattern = QRegularExpression(QStringLiteral("\\b[0-9\\.]+\\b"));
        rule.format = numberFormat;
        mHighlightingRules.append(rule);

        mErrorFormat.setForeground(Qt::red);

        mBracketsHighlightFormat.setBackground(QColor(0, 125, 200));

        mObjectsExpression = QRegularExpression(QStringLiteral(
                             "\\b"
                             "(?!sin\\(|cos\\(|tan\\(|exp\\(|"
                                "asin\\(|acos\\(|atan\\()"
                             "([A-Za-z_][A-Za-z0-9_]*\\.?)+"));
    }

    void setCursorPos(const int pos) {
        mCursorPos = pos;
        setDocument(document());
    }
protected:
    void highlightBlock(const QString &text) override;
private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    int mCursorPos = 0;
    QTextCharFormat mBracketsHighlightFormat;
    QTextCharFormat mErrorFormat;
    QVector<HighlightingRule> mFuncRules;
    QVector<HighlightingRule> mHighlightingRules;

    QRegularExpression mObjectsExpression;

    QrealAnimator* const mTarget;
    ComplexAnimator* const mSearchCtxt;
    ExpressionEditor* const mEditor;
};

class ExpressionEditor : public QTextEdit {
public:
    ExpressionEditor(QrealAnimator* const target,
                     QWidget* const parent) :
        ExpressionEditor(target, target->expressionText(), parent) {}

    ExpressionEditor(QrealAnimator* const target,
                     const QString& text, QWidget* const parent) :
        QTextEdit(text, parent) {
        setTextColor(Qt::red);
        setMinimumWidth(400);
        const auto doc = document();
        doc->setMaximumBlockCount(1);
        setAcceptRichText(false);
        const auto highligter = new Highlighter(target, this, doc);
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
                         QOverload<const QString &>::of(&QCompleter::activated),
                         this, &ExpressionEditor::insertCompletion);
    }

    void setCompleterList(const QStringList& values) {
        mCompleter->setModel(new QStringListModel(values, mCompleter));
        const auto popup = mCompleter->popup();
        if(values.isEmpty()) return popup->hide();
        mCompleter->setCompletionPrefix(textUnderCursor());
        QRect cr = cursorRect();
        cr.setWidth(popup->sizeHintForColumn(0)
                    + popup->verticalScrollBar()->sizeHint().width());
        mCompleter->complete(cr);
    }
protected:
    void keyPressEvent(QKeyEvent *e) override {
        if(e->key() == Qt::Key_Return) return;
        QTextEdit::keyPressEvent(e);
    }
private:
    void insertCompletion(const QString &completion) {
        QTextCursor tc = textCursor();
        const int prefixLen = mCompleter->completionPrefix().length();
        const int extra = completion.length() - prefixLen;
        tc.movePosition(QTextCursor::Left);
        tc.movePosition(QTextCursor::EndOfWord);
        tc.insertText(completion.right(extra));
        setTextCursor(tc);
    }

    QString textUnderCursor() const {
        QTextCursor tc = textCursor();
        tc.select(QTextCursor::WordUnderCursor);
        return tc.selectedText();
    }

    QCompleter* mCompleter;
};


void Highlighter::highlightBlock(const QString &text) {
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

    mEditor->setCompleterList(QStringList());
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
            const auto lastObjPath = subPath.last();
            const int max = min + lastObjPath.count();
            const bool autocomplete = mCursorPos > min && mCursorPos <= max;
            QStringList completions;
            const auto obj = mSearchCtxt->ca_findPropertyWithPathRecBothWays(
                        0, subPath, autocomplete ? &completions : nullptr);
            if(autocomplete) {
                completions.removeDuplicates();
                mEditor->setCompleterList(completions);
            }

            if(!obj || obj == mTarget) {
                const int len = match.capturedEnd() - min;
                setFormat(min, len, mErrorFormat);
                break;
            }
        }
    }
}

ExpressionDialog::ExpressionDialog(QrealAnimator* const target,
                                   QWidget * const parent) :
    QDialog(parent), mTarget(target) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Expression " + target->prp_getName());

    const auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

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
        if(!expr) return;
        expr->setRelFrame(0);
        expr->collapse();
        if(!expr->isValid()) {
            mTarget->clearExpression();
            RuntimeThrow("Invalid expression.");
        } else mTarget->setExpression(text, expr);
        Document::sInstance->actionFinished();
    } catch(const std::exception& e) {
        const QString error = e.what();
        const QString lastLine = error.split(QRegExp("\n|\r\n|\r")).last();
        mErrorLabel->setText(lastLine);
    }
}
