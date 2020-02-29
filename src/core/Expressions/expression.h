#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QObject>
#include <QJSEngine>

#include "propertybindingparser.h"

class Expression : public QObject {
    Q_OBJECT
    Expression(const QString& definitionsStr,
               const QString& scriptStr,
               PropertyBindingMap&& bindings,
               std::unique_ptr<QJSEngine>&& engine,
               QJSValue&& eEvaluate);
public:
    static void sAddDefinitionsTo(const QString& definitionsStr,
                                  QJSEngine& e);
    using ResultTester = std::function<void(const QJSValue&)>;
    static void sAddScriptTo(const QString& scriptStr,
                             const PropertyBindingMap& bindings,
                             QJSEngine& e, QJSValue& eEvaluate,
                             const ResultTester& resultTester);
    static qsptr<Expression> sCreate(const QString& definitionsStr,
                                     const QString& scriptStr,
                                     PropertyBindingMap&& bindings,
                                     std::unique_ptr<QJSEngine>&& engine,
                                     QJSValue&& eEvaluate);
    static qsptr<Expression> sCreate(const QString& bindingsStr,
                                     const QString& definitionsStr,
                                     const QString& scriptStr,
                                     const Property* const context,
                                     const ResultTester& resultTester);

    static ResultTester sQrealAnimatorTester;

    bool setAbsFrame(const qreal absFrame);

    bool isValid();
    bool dependsOn(const Property* const prop);

    QJSValue evaluate();
    QJSValue evaluate(const qreal relFrame);

    FrameRange identicalRange(const qreal absFrame);

    QString bindingsString() const;
    const QString& definitionsString() const { return mDefinitionsStr; }
    const QString& scriptString() const { return mScriptStr; }
signals:
    void relRangeChanged(const FrameRange& range);
    void currentValueChanged();
private:
    const QString mDefinitionsStr;
    const QString mScriptStr;

    QJSValue mEEvaluate;
    const PropertyBindingMap mBindings;
    const std::unique_ptr<QJSEngine> mEngine;
};

#endif // EXPRESSION_H
