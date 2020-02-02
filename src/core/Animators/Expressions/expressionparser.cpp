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

#include "expressionparser.h"

#include "exceptions.h"

#include "expressionplainvalue.h"
#include "expressionoperator.h"
#include "expressionfunction.h"
#include "expressionrandomfunction.h"
#include "expressionsource.h"
#include "expressionsourceframe.h"
#include "expressionsourcevalue.h"
#include "expressionnegatefunction.h"

void skipSpaces(const QString& exp, int& position) {
    while(position < exp.count() && exp.at(position) == ' ') {
        position++;
    }
}

bool parseExpression(const QString& exp,
                     int& position,
                     QString& expression) {
    int newPosition = position;
    expression.clear();
    bool first = true;
    for(; newPosition < exp.length(); newPosition++) {
        const auto& character = exp.at(newPosition);
        const bool valid = character.isLetter() ||
                           character == '_' ||
                           (character.isNumber() && !first) ||
                           (character == ' ' && !first) ||
                           character == '.';
        if(valid) expression.append(character);
        else break;
        first = false;
    }
    if(expression.isEmpty()) return false;
    expression = expression.trimmed();
    position = newPosition;
    return true;
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

bool parseOperator(const QString& exp,
                   int& position,
                   Operator& oper) {
    int newPosition = position;
    oper = idOperator(exp.at(position));
    if(oper == Operator::notOperator) return false;
    newPosition++;
    position = newPosition;
    return true;
}

enum class Special {
    value, frame
};

bool parseSpecial(const QString& exp,
                  int& position,
                  Special& spec) {
    int newPosition = position;
    const auto part = exp.mid(position, 6);
    if(part == "$value") {
        spec = Special::value;
    } else if(part == "$frame") {
        spec = Special::frame;
    } else return false;
    newPosition += 6;
    position = newPosition;
    return true;
}

enum class Function {
    sin, cos, tan, exp,
    asin, acos, atan,
    sqrt, rand
};

bool parseFunction(const QString& exp,
                   int& position,
                   Function& function) {
    if(position + 5 >= exp.length()) return false;
    int newPosition = position;
    const auto str4 = exp.mid(newPosition, 4);
    const auto str5 = exp.mid(newPosition, 5);
    if(str4 == "sin(") {
        function = Function::sin;
        newPosition += 3;
    } else if(str4 == "cos(") {
        function = Function::cos;
        newPosition += 3;
    } else if(str4 == "tan(") {
        function = Function::tan;
        newPosition += 3;
    } else if(str4 == "exp(") {
        function = Function::exp;
        newPosition += 3;
    } else if(str5 == "asin(") {
        function = Function::asin;
        newPosition += 4;
    } else if(str5 == "acos(") {
        function = Function::acos;
        newPosition += 4;
    } else if(str5 == "atan(") {
        function = Function::atan;
        newPosition += 4;
    } else if(str5 == "sqrt(") {
        function = Function::sqrt;
        newPosition += 4;
    } else if(str5 == "rand(") {
        function = Function::rand;
        newPosition += 4;
    } else return false;
    position = newPosition;
    return true;
}

bool parseBrackets(const QString& exp,
                   int& position,
                   QString& contained) {
    if(position >= exp.count()) return false;
    int newPosition = position;
    if(exp.at(newPosition) != '(') return false;
    newPosition++;
    contained.clear();
    bool foundClosing = false;
    int containedBrackets = 0;
    for(; newPosition < exp.length(); newPosition++) {
        const auto& character = exp.at(newPosition);
        if(character == '(') containedBrackets++;
        else if(character == ')') {
            if(containedBrackets == 0) {
                newPosition++;
                foundClosing = true;
                break;
            }
            containedBrackets--;
        }
        contained.append(character);
    }
    if(containedBrackets > 0 || !foundClosing)
        RuntimeThrow("Missing closing brackets.");
    position = newPosition;
    return true;
}

bool parsePlainValue(const QString& exp,
                     int& position,
                     qreal& value,
                     QString& parsed) {
    if(position >= exp.count()) return false;
    parsed.clear();
    int newPosition = position;

    bool separator = false;
    for(; newPosition < exp.length(); newPosition++) {
        const auto& character = exp.at(newPosition);
        const bool charSeparator = character == '.' || character == ',';
        if(charSeparator) {
            if(separator) break;
            separator = true;
        } else if(!character.isNumber()) break;
        parsed.append(character);
    }

    if(parsed.isEmpty()) return false;
    bool ok;
    value = parsed.toDouble(&ok);
    Q_ASSERT(ok);
    position = newPosition;
    return true;
}

QSharedPointer<ExpressionValue> createOperator(
            const Operator oper,
            const QSharedPointer<ExpressionValue>& opValue1,
            const QSharedPointer<ExpressionValue>& opValue2) {
    if(!opValue1) RuntimeThrow("Missing value before operator.");
    if(!opValue2) RuntimeThrow("Missing value after operator.");
    std::function<qreal(qreal, qreal)> func;
    QString symbol;
    bool needsBrackets;
    bool childrenNeedBrackets;
    switch(oper) {
    case Operator::multiply:
        needsBrackets = false;
        childrenNeedBrackets = true;
        symbol = "*";
        func = [](qreal v1, qreal v2) { return v1 * v2; };
        break;
    case Operator::exponate:
        needsBrackets = false;
        childrenNeedBrackets = true;
        symbol = "^";
        func = [](qreal v1, qreal v2) { return std::pow(v1, v2); };
        break;
    case Operator::modulo:
        needsBrackets = false;
        childrenNeedBrackets = true;
        symbol = "%";
        func = [](qreal v1, qreal v2) { return std::fmod(v1, v2); };
        break;
    case Operator::divide:
        needsBrackets = false;
        childrenNeedBrackets = true;
        symbol = "/";
        func = [](qreal v1, qreal v2) { return v1 / v2; };
        break;
    case Operator::add:
        needsBrackets = true;
        childrenNeedBrackets = false;
        symbol = "+";
        func = [](qreal v1, qreal v2) { return v1 + v2; };
        break;
    case Operator::subtract:
        needsBrackets = true;
        childrenNeedBrackets = false;
        symbol = "-";
        func = [](qreal v1, qreal v2) { return v1 - v2; };
        break;
    case Operator::notOperator: return nullptr;
    }
    return ExpressionOperator::sCreate(childrenNeedBrackets, needsBrackets,
                                       symbol, func, opValue1, opValue2);
}

QSharedPointer<ExpressionValue> createFunction(
            const QString& exp, QrealAnimator * const parent,
            int& position, const Function function) {
    QString parsed;
    if(position >= exp.count() || !parseBrackets(exp, position, parsed))
        RuntimeThrow("Missing Function parameter.");
    const auto inner = ExpressionParser::parse(parsed, parent);
    if(!inner) RuntimeThrow("Missing Function parameter.");
    std::function<qreal(qreal)> func;
    QString name;
    switch(function) {
    case Function::sin:
        name = "sin";
        func = [](qreal v) { return std::sin(v); };
        break;
    case Function::cos:
        name = "cos";
        func = [](qreal v) { return std::cos(v); };
        break;
    case Function::tan:
        name = "tan";
        func = [](qreal v) { return std::tan(v); };
        break;
    case Function::exp:
        name = "exp";
        func = [](qreal v) { return std::exp(v); };
        break;
    case Function::asin:
        name = "asin";
        func = [](qreal v) { return std::asin(v); };
        break;
    case Function::acos:
        name = "acos";
        func = [](qreal v) { return std::acos(v); };
        break;
    case Function::atan:
        name = "atan";
        func = [](qreal v) { return std::atan(v); };
        break;
    case Function::sqrt:
        name = "sqrt";
        func = [](qreal v) { return std::sqrt(v); };
        break;
    case Function::rand:
        return ExpressionRandomFunction::sCreate(inner);
        break;
    }
    return ExpressionFunction::sCreate(name, func, inner);
}

QSharedPointer<ExpressionValue> ExpressionParser::parse(
        const QString& exp, QrealAnimator* const parent) {
    QString parsed;
    Special spec;
    Operator oper = Operator::notOperator;
    Function func;
    qreal value;
    QSharedPointer<ExpressionValue> opValue1;
    QSharedPointer<ExpressionValue> opValue2;
    bool negate = false;

    const auto setValue = [&opValue1, &opValue2](
            const QSharedPointer<ExpressionValue>& value) {
        if(opValue1) opValue2 = value;
        else opValue1 = value;
    };

    for(int i = 0; i < exp.count();) {
        Operator operTmp;
        skipSpaces(exp, i);
        if(i >= exp.count()) break;
        QSharedPointer<ExpressionValue> lastValue;
        if(parseBrackets(exp, i, parsed)) {
            lastValue = parse(parsed, parent);
        } else if(parsePlainValue(exp, i, value, parsed)) {
            lastValue = ExpressionPlainValue::sCreate(parsed, value);
        } else if(parseOperator(exp, i, operTmp)) {
            bool negator = false;
            if(!opValue1) {
                if(operTmp == Operator::subtract) {
                    negator = true;
                } else RuntimeThrow("Missing value before operator");
            } else if(oper != Operator::notOperator) {
                if(operTmp == Operator::subtract) {
                    negator = true;
                } else RuntimeThrow("Two operators in row " + exp.mid(i));
            }
            if(negator) negate = !negate;
            else oper = operTmp;
            continue;
        } else if(parseFunction(exp, i, func)) {
            lastValue = createFunction(exp, parent, i, func);
        } else if(parseSpecial(exp, i, spec)) {
            if(parent) {
                switch(spec) {
                case Special::value:
                    lastValue = ExpressionSourceValue::sCreate(parent);
                    break;
                case Special::frame:
                    lastValue = ExpressionSourceFrame::sCreate(parent);
                    break;
                }
            } else {
                lastValue = ExpressionPlainValue::sCreate(1);
            }
        } else if(parseExpression(exp, i, parsed)) {
            if(parent) {
                lastValue = ExpressionSource::sCreate(parsed, parent);
            } else {
                lastValue = ExpressionPlainValue::sCreate(1);
            }
        } else {
            if(exp.at(i) == ')') RuntimeThrow("Unexpected closing bracket.");
            RuntimeThrow("Invalid expression " + exp.mid(i));
        }
        if(lastValue) {
            if(negate) {
                lastValue = ExpressionNegateFunction::sCreate(lastValue);
                negate = false;
            }
            setValue(lastValue);
        }
        if(oper == Operator::notOperator && opValue1 && opValue2)
            oper = Operator::multiply;
        if(oper != Operator::notOperator) {
            opValue1 = createOperator(oper, opValue1, opValue2);
            opValue2.reset();
            oper = Operator::notOperator;
        }
    }
    return opValue1;
}
