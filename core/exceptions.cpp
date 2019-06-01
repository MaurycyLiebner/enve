#include "exceptions.h"
#include <QMessageBox>

std::string operator+(const std::string& c, const QString& k) {
    return c + k.toStdString();
}

std::string operator<<(const std::string& c, const QString& k) {
    return c + k.toStdString();
}

std::string operator>>(const QString& k, const std::string& c) {
    return k.toStdString() + c;
}

QDebug operator<<(QDebug out, const std::string& str) {
    out << str.c_str();
    return out;
}

bool isExceptionNested(const std::exception& e) {
    if(auto ne = dynamic_cast<const std::nested_exception*>(std::addressof(e))) {
        if(ne->nested_ptr()) return true;
    }
    return false;
}

void _gPrintException(const std::exception& e,
                      QString allText,
                      const uint& level,
                      const bool& fatal) {
    allText = QString::number(level) + ") " + e.what() + "\n " + allText;
    qCritical() << std::to_string(level) + ") " << e.what();
    try {
        if(!isExceptionNested(e)) {
            gPrintException(fatal, allText);
            if(!fatal) return;
        }
        std::rethrow_if_nested(e);
    } catch(const std::exception& ne) {
        _gPrintException(ne, allText, level + 1, fatal);
    } catch(...) {}
}

void gPrintExceptionCritical(const std::exception& e) {
    _gPrintException(e, "", 0, false);
}

void gPrintExceptionFatal(const std::exception& e) {
    _gPrintException(e, "", 0, true);
}

QString gAllTextFromException(const std::exception &e,
                              QString allText,
                              const uint &level) {
    allText = (allText.isEmpty() ? "" : allText + "\n") +
            (std::to_string(level) + ") ").c_str() + e.what();
    qCritical() << std::to_string(level) + ") " << e.what();
    try {
        if(!isExceptionNested(e))  return allText;
        std::rethrow_if_nested(e);
    } catch(const std::exception& ne) {
        gAllTextFromException(ne, allText, level + 1);
    } catch(...) {}
    return allText;
}

void gPrintException(const bool& fatal, const QString &allText) {
    const QString txt = fatal ? "Fatal" : "Critical";
    QMessageBox(QMessageBox::Critical, txt + " Error", allText).exec();
}

void gPrintExceptionCritical(const std::exception_ptr &eptr) {
    try {
        if(eptr) std::rethrow_exception(eptr);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void gPrintExceptionFatal(const std::exception_ptr &eptr) {
    try {
        if(eptr) std::rethrow_exception(eptr);
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
    }
}
