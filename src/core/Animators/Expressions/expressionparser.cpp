#include "expressionparser.h"

#include "exceptions.h"

#include "expressionplainvalue.h"
#include "expressionoperator.h"
#include "expressionfunction.h"
#include "expressionsource.h"

QSharedPointer<ExpressionValue> parsePrepared(
        const QString &exp, QrealAnimator* const parent);

bool parseExpression(const QString& exp, int& position,
                     QString& expression) {
    expression.clear();
    bool first = true;
    for(; position < exp.length(); position++) {
        const auto& character = exp.at(position);
        const bool valid = character.isLetter() ||
                           character == '_' ||
                           (character.isNumber() && !first) ||
                           character == '.';
        if(valid) expression.append(character);
        else break;
        first = false;
    }
    return !expression.isEmpty();
}

enum class Operator {
    multiply,
    exponate,
    modulo,
    divide,
    add,
    subtract,
    notOperator
};

Operator idOperator(const QChar& character) {
    if(character == '*') return Operator::multiply;
    if(character == '^') return Operator::exponate;
    if(character == '%') return Operator::modulo;
    if(character == '/') return Operator::divide;
    if(character == '+') return Operator::add;
    if(character == '-') return Operator::subtract;
    else return Operator::notOperator;
}

bool parseOperator(const QString& exp, int& position,
                   Operator& oper) {
    oper = idOperator(exp.at(position));
    if(oper == Operator::notOperator) return false;
    position++;
    return true;
}

enum class Function {
    sin, cos, tan, exp,
    asin, acos, atan,
};

bool parseFunction(const QString& exp, int& position,
                   Function& function) {
    if(position + 5 >= exp.length()) return false;
    const auto str4 = exp.mid(position, 4);
    const auto str5 = exp.mid(position, 5);
    if(str4 == "sin(") {
        function = Function::sin;
        position += 3;
    } else if(str4 == "cos(") {
        function = Function::cos;
        position += 3;
    } else if(str4 == "tan(") {
        function = Function::tan;
        position += 3;
    } else if(str4 == "exp(") {
        function = Function::exp;
        position += 3;
    } else if(str5 == "asin(") {
        function = Function::asin;
        position += 4;
    } else if(str5 == "acos(") {
        function = Function::acos;
        position += 4;
    } else if(str5 == "atan(") {
        function = Function::atan;
        position += 4;
    } else return false;
    return true;
}

bool parseBrackets(const QString& exp, int& position,
                   QString& contained) {
    if(exp.at(position) != '(') return false;
    position++;
    contained.clear();
    bool foundClosing = false;
    int containedBrackets = 0;
    for(; position < exp.length(); position++) {
        const auto& character = exp.at(position);
        if(character == '(') containedBrackets++;
        else if(character == ')') {
            if(containedBrackets == 0) {
                position++;
                foundClosing = true;
                break;
            }
            containedBrackets--;
        }
        contained.append(character);
    }
    if(containedBrackets > 0 || !foundClosing)
        RuntimeThrow("Missing closing brackets.");
    return true;
}

bool parsePlainValue(const QString& exp, int& position,
                     qreal& value) {
    QString valueStr;

    bool separator = false;
    for(; position < exp.length(); position++) {
        const auto& character = exp.at(position);
        const bool charSeparator = character == '.' || character == ',';
        if(charSeparator) {
            if(separator) break;
            separator = true;
        }
        if(!character.isNumber()) break;
        valueStr.append(character);
    }

    if(valueStr.isEmpty()) return false;
    bool ok;
    value = valueStr.toDouble(&ok);
    Q_ASSERT(ok);
    return ok;
}

bool parseImplicitMultiplication(const QString& exp, int& position,
                                 qreal& value) {
    const int iniPosition = position;
    const bool isNumeric = parsePlainValue(exp, position, value);
    if(isNumeric && position < exp.count()) {
        const auto& character = exp.at(position);
        if(character.isLetter()) return true;
    }
    position = iniPosition;
    return false;
}

QSharedPointer<ExpressionValue> createOperator(
            const QString& exp, QrealAnimator * const parent,
            int& position, const Operator oper,
            const QSharedPointer<ExpressionValue>& opValue1) {
    if(!opValue1) RuntimeThrow("Operator needs a value before it.");
    const auto opValue2 = parsePrepared(exp.mid(position), parent);
    if(!opValue2) RuntimeThrow("Operator needs a value after it.");
    position = exp.count();
    std::function<qreal(qreal, qreal)> func;
    switch(oper) {
    case Operator::multiply:
        func = [](qreal v1, qreal v2) { return v1 * v2; };
        break;
    case Operator::exponate:
        func = [](qreal v1, qreal v2) { return std::pow(v1, v2); };
        break;
    case Operator::modulo:
        func = [](qreal v1, qreal v2) { return std::fmod(v1, v2); };
        break;
    case Operator::divide:
        func = [](qreal v1, qreal v2) { return v1 / v2; };
        break;
    case Operator::add:
        func = [](qreal v1, qreal v2) { return v1 + v2; };
        break;
    case Operator::subtract:
        func = [](qreal v1, qreal v2) { return v1 - v2; };
        break;
    case Operator::notOperator: return nullptr;
    }
    return ExpressionOperator::sCreate(func, opValue1, opValue2);
}

QSharedPointer<ExpressionValue> createFunction(
            const QString& exp, QrealAnimator * const parent,
            int& position, const Function function) {
    QString parsed;
    if(position >= exp.count() || !parseBrackets(exp, position, parsed))
        RuntimeThrow("Missing Function parameter.");
    const auto inner = parsePrepared(parsed, parent);
    if(!inner) RuntimeThrow("Missing Function parameter.");
    std::function<qreal(qreal)> func;
    switch(function) {
    case Function::sin:
        func = [](qreal v) { return std::sin(v); };
        break;
    case Function::cos:
        func = [](qreal v) { return std::cos(v); };
        break;
    case Function::tan:
        func = [](qreal v) { return std::tan(v); };
        break;
    case Function::exp:
        func = [](qreal v) { return std::exp(v); };
        break;
    case Function::asin:
        func = [](qreal v) { return std::asin(v); };
        break;
    case Function::acos:
        func = [](qreal v) { return std::acos(v); };
        break;
    case Function::atan:
        func = [](qreal v) { return std::atan(v); };
        break;
    }
    return ExpressionFunction::sCreate(func, inner);
}

QSharedPointer<ExpressionValue> parsePrepared(
        const QString& exp, QrealAnimator* const parent) {
    QString parsed;
    Operator oper;
    Function func;
    qreal value;
    QSharedPointer<ExpressionValue> opValue1;

    for(int i = 0; i < exp.count();) {
        if(parseBrackets(exp, i, parsed)) {
            opValue1 = parsePrepared(parsed, parent);
        } else if(parsePlainValue(exp, i, value)) {
            opValue1 = ExpressionPlainValue::sCreate(value);
            if(i < exp.count()) {
                const auto& character = exp.at(i);
                if(character.isLetter() || character == '_' || character == '(') {
                    opValue1 = createOperator(exp, parent, i, Operator::multiply, opValue1);
                }
            }
        } else if(parseOperator(exp, i, oper)) {
            opValue1 = createOperator(exp, parent, i, oper, opValue1);
        } else if(parseFunction(exp, i, func)) {
            opValue1 = createFunction(exp, parent, i, func);
        } else if(parseExpression(exp, i, parsed)) {
            opValue1 = ExpressionSource::sCreate(parsed, parent);
        } else {
            if(exp.at(i) == ')') RuntimeThrow("Unexpected closing bracket.");
            RuntimeThrow("Invalid expression " + exp.mid(i));
        }
    }
    return opValue1;
}


QSharedPointer<ExpressionValue> ExpressionParser::parse(
        QString exp, QrealAnimator* const parent) {
    exp.remove(' ');
    return parsePrepared(exp, parent);
}
