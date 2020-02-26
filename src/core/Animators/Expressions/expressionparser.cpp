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
#include "expressioncomplex.h"

using ExprSPtr = QSharedPointer<ExpressionValue>;
using VarSPtr = QSharedPointer<ExpressionVariable>;
using PlainVarSPtr = QSharedPointer<ExpressionPlainVariable>;

using VariableMap = std::map<QString, ExprSPtr>;
using PlainVarMap = std::map<QString, PlainVarSPtr>;

ExprSPtr parse(const QString& exp, int& i,
               QrealAnimator* const parent,
               VariableMap& variables,
               const bool stopAtBreak,
               const ExpressionType type);

void skipSpaces(const QString& exp, int& position) {
    while(position < exp.count() && exp.at(position) == ' ') {
        position++;
    }
}

bool parseExpression(const QString& exp, int& position,
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

bool parseSpecial(const QString& exp, int& position,
                  QString& parsed) {
    if(exp.at(position) != '$') return false;
    int newPosition = position + 1;
    parsed = "$";
    for(;newPosition < exp.count(); newPosition++) {
        const auto& character = exp.at(newPosition);
        if(!character.isLetter() && !character.isDigit()
           && character != '_') break;
        parsed.append(character);
    }
    position = newPosition;
    return true;
}

bool parseSettingSpecial(const QString& exp, int& position,
                         QString& parsed) {
    if(exp.at(position) != '$') return false;
    int newPosition = position + 1;
    parsed = "$";
    while(newPosition < exp.count()) {
        const auto& character = exp.at(newPosition++);
        if(!character.isLetter() && !character.isDigit()
           && character != '_') break;
        parsed.append(character);
    }
    skipSpaces(exp, newPosition);
    if(newPosition < exp.count()) {
        const auto& character = exp.at(newPosition++);
        if(character != '=') return false;
    } else return false;
    position = newPosition;
    return true;
}

bool parsePlainVariable(const QString& exp, int& position,
                         QString& parsed) {
    if(exp.at(position) != '$') return false;
    int newPosition = position + 1;
    parsed = "$";
    while(newPosition < exp.count()) {
        const auto& character = exp.at(newPosition++);
        if(!character.isLetter() && !character.isDigit()
           && character != '_') break;
        parsed.append(character);
    }
    skipSpaces(exp, newPosition);
    if(newPosition < exp.count()) {
        const auto& character = exp.at(newPosition++);
        if(character != ';') return false;
    } else return false;
    position = newPosition;
    return true;
}

enum class Function {
    sin, cos, tan, exp, abs,
    asin, acos, atan,
    sqrt, rand
};

bool parseFunction(const QString& exp, int& position,
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
    } else if(str4 == "abs(") {
        function = Function::abs;
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

bool parseBrackets(const QString& exp, int& position,
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

bool parsePlainValue(const QString& exp, int& position,
                     qreal& value, QString& parsed) {
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
            parsed.append('.');
        } else if(!character.isNumber()) break;
        else parsed.append(character);
    }

    if(parsed.isEmpty()) return false;
    bool ok;
    value = parsed.toDouble(&ok);
    Q_ASSERT(ok);
    position = newPosition;
    return true;
}

ExprSPtr createOperator(const Operator oper,
                        const ExprSPtr& opValue1,
                        const ExprSPtr& opValue2) {
    if(!opValue1) RuntimeThrow("Missing value before operator.");
    if(!opValue2) RuntimeThrow("Missing value after operator.");
    std::function<qreal(qreal, qreal)> func;
    QString symbol;
    bool needsBrackets = false;
    bool childrenNeedBrackets = false;
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

ExprSPtr createFunction(const QString& exp, int& position,
                        QrealAnimator * const parent,
                        VariableMap& variables,
                        const Function function,
                        const ExpressionType type) {
    QString parsed;
    if(position >= exp.count() || !parseBrackets(exp, position, parsed))
        RuntimeThrow("Missing Function parameter.");
    int pos = 0;
    const auto inner = parse(parsed, pos, parent, variables, false, type);
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
    case Function::abs:
        name = "abs";
        func = [](qreal v) { return std::abs(v); };
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

ExprSPtr parse(const QString& exp, int& i,
               QrealAnimator* const parent,
               VariableMap& variables,
               const bool stopAtBreak,
               const ExpressionType type) {
    QString parsed;
    Operator oper = Operator::notOperator;
    Function func;
    qreal value;
    ExprSPtr opValue1;
    ExprSPtr opValue2;
    bool negate = false;
    const bool sourceVarsAllowed = type == ExpressionType::allFeatures ||
                                   type == ExpressionType::noPlainVariables;

    const auto setValue = [&opValue1, &opValue2](const ExprSPtr& value) {
        if(opValue1) opValue2 = value;
        else opValue1 = value;
    };

    while(i < exp.count()) {
        Operator operTmp;
        skipSpaces(exp, i);
        if(i >= exp.count()) break;
        ExprSPtr lastValue;
        if(parseBrackets(exp, i, parsed)) {
            lastValue = ExpressionParser::parse(parsed, parent, type);
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
            if(negator) {
                negate = !negate;
                continue;
            } else {
                oper = operTmp;
                const bool addSub = oper == Operator::subtract ||
                                    oper == Operator::add;
                if(addSub) {
                    if(stopAtBreak) {
                        i--;
                        return opValue1;
                    } else {
                        opValue2 = parse(exp, i, parent, variables, true, type);
                    }
                } else {
                    continue;
                }
            }

        } else if(parseFunction(exp, i, func)) {
            lastValue = createFunction(exp, i, parent, variables, func, type);
        } else if(parseSpecial(exp, i, parsed)) {
            const auto it = variables.find(parsed);
            if(it != variables.end()) {
                lastValue = it->second;
            } else RuntimeThrow("Invalid variable.");
        } else if(sourceVarsAllowed && parseExpression(exp, i, parsed)) {
            if(parent) {
                lastValue = ExpressionSource::sCreate(parsed, parent);
            } else RuntimeThrow("No context to look for objects in.");
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
        if(oper == Operator::notOperator && opValue1 && opValue2) {
            oper = Operator::multiply;
        }
        if(oper != Operator::notOperator) {
            opValue1 = createOperator(oper, opValue1, opValue2);
            opValue2.reset();
            oper = Operator::notOperator;
        }
    }
    return opValue1;
}

ExprSPtr ExpressionParser::parse(QString exp, QrealAnimator* const parent,
                                 const ExpressionType type) {
    const bool plainVarsAllowed = type == ExpressionType::allFeatures ||
                                  type == ExpressionType::noSourceVariables;
    const bool sourceVarsAllowed = type == ExpressionType::allFeatures ||
                                   type == ExpressionType::noPlainVariables;

    exp.remove(QRegExp("\n|\r\n|\r"));
    const auto lines = exp.split(';', QString::SkipEmptyParts);
    PlainVarMap plainVariables;
    VariableMap variables;
    ExprSPtr frame;
    ExprSPtr value;
    if(sourceVarsAllowed && parent) {
        frame = ExpressionSourceFrame::sCreate(parent);
        value = ExpressionSourceValue::sCreate(parent);
        variables["$frame"] = frame;
        variables["$value"] = value;
    }
    ExprSPtr lastValue;
    for(const auto& line : lines) {
        int pos = 0;
        QString varName;
        if(plainVarsAllowed && parsePlainVariable(line, pos, varName)) {
            const auto var = ExpressionPlainVariable::sCreate(varName);
            plainVariables[varName] = var;
        } else if(parseSettingSpecial(line, pos, varName)) {
            const auto definition = parse(line, pos, parent, variables, false, type);
            if(!definition) RuntimeThrow("Missing function definition.");
            const auto var = ExpressionVariable::sCreate(varName, definition);
            variables[varName] = var;
        } else lastValue = parse(line, pos, parent, variables, false, type);
    }
    if(!lastValue) RuntimeThrow("No return value.");
    QList<VarSPtr> sourceVarList;
    for(auto it = variables.begin(); it != variables.end(); it++) {
        const auto& var = it->second;
        if(var == frame || var == value) continue;
        sourceVarList << qSharedPointerCast<ExpressionVariable>(var);
    }

    QList<PlainVarSPtr> plainVarList;
    if(plainVarsAllowed) {
        for(auto it = plainVariables.begin(); it != plainVariables.end(); it++) {
            plainVarList << it->second;
        }
    }

    return ExpressionComplex::sCreate(plainVarList, sourceVarList, lastValue);
}
