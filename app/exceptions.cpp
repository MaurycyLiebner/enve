#include "exceptions.h"
#include <QMessageBox>

QDebug operator<<(QDebug out, const std::string& str) {
    out << str.c_str();
    return out;
}

bool isExceptionNested(const std::exception& e) {
    if(auto ne = dynamic_cast<const std::nested_exception*>(std::addressof(e))) {
        if(ne->nested_ptr()) {
            return true;
        }
    }
    return false;
}

void _gPrintException(const std::exception& e,
                      QString allText,
                      const uint& level,
                      const bool& fatal) {
    if(allText.isEmpty()) allText = (std::to_string(level) + ") " + e.what()).c_str();
    qCritical() << std::to_string(level) + ") " << e.what();
    try {
        if(!isExceptionNested(e)) {
            QMessageBox(QMessageBox::Critical, "Critical Error", allText).exec();
            if(!fatal) return;
        }
        std::rethrow_if_nested(e);
    } catch(const std::exception& ne) {
        allText = allText + "\n" + (std::to_string(level + 1) + ") ").c_str() + ne.what();
        _gPrintException(ne, allText, level + 1, fatal);
    } catch(...) {}
}

void gPrintExceptionCritical(const std::exception& e,
                     QString allText,
                     const uint& level) {
    _gPrintException(e, allText, level, false);
}

void gPrintExceptionFatal(const std::exception& e,
                             QString allText,
                             const uint& level) {
    _gPrintException(e, allText, level, true);
}
