#include "expressiondialog.h"

#include <QLabel>
#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QCheckBox>
#include <QStringListModel>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStatusBar>
#include <QApplication>

#include "Animators/qrealanimator.h"
#include "Animators/Expressions/expressionparser.h"
#include "GUI/global.h"
#include "Boxes/boundingbox.h"
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

        const QStringList funcs = {
            QStringLiteral("sin"),
            QStringLiteral("cos"),
            QStringLiteral("tan"),
            QStringLiteral("asin"),
            QStringLiteral("acos"),
            QStringLiteral("atan"),
            QStringLiteral("exp"),
            QStringLiteral("sqrt"),
            QStringLiteral("rand")
        };

        QTextCharFormat funcFormat;
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
    QVector<HighlightingRule> mHighlightingRules;
    QVector<HighlightingRule> mFuncRules;

    QRegularExpression mObjectsExpression;

    QStringList mBaseComplete;

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
                         qOverload<const QString&>(&QCompleter::activated),
                         this, &ExpressionEditor::insertCompletion);
    }

    void setCompleterList(const QStringList& values) {
        mCompleter->setModel(new QStringListModel(values, mCompleter));
    }
protected:
    void keyPressEvent(QKeyEvent *e) override {
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


private:
    void showCompleter() {
        const auto popup = mCompleter->popup();
        const auto underCursor = textUnderCursor();
        mCompleter->setCompletionPrefix(underCursor);
        QRect cr = cursorRect();
        const auto scrollBar = popup->verticalScrollBar();
        const int scrollBarWidth = scrollBar->sizeHint().width();
        cr.setWidth(popup->sizeHintForColumn(0) + scrollBarWidth);
        mCompleter->complete(cr);
    }

    void insertCompletion(const QString &completion) {
        QTextCursor tc = textCursor();
        const int prefixLen = mCompleter->completionPrefix().length();
        const int extra = completion.length() - prefixLen;
        tc.movePosition(QTextCursor::Left);
        tc.movePosition(QTextCursor::EndOfWord);
        tc.insertText(completion.right(extra));
        if(completion.right(1) == ')') tc.movePosition(QTextCursor::Left);
        setTextCursor(tc);
    }

    QString textUnderCursor() const {
        QTextCursor tc = textCursor();
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
            }

            if(!obj || obj == mTarget) {
                const int len = match.capturedEnd() - min;
                setFormat(min, len, mErrorFormat);
                break;
            }
        }
    }
    if(addFuncsComplete) {
        QStringList tmp = mBaseComplete;
        tmp.append(completions);
        completions = tmp;
    }
    mEditor->setCompleterList(completions);
}

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
        if(!expr) return;
        expr->setRelFrame(0);
        //expr->collapse();
        if(!expr->isValid()) {
            mTarget->clearExpression();
            RuntimeThrow("Invalid expression.");
        } else mTarget->setExpression(expr);
        Document::sInstance->actionFinished();
    } catch(const std::exception& e) {
        const QString error = e.what();
        const QString lastLine = error.split(QRegExp("\n|\r\n|\r")).last();
        mErrorLabel->setText(lastLine);
    }
}
