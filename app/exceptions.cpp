#include "exceptions.h"

QDebug operator<<(QDebug out, const std::string& str) {
    out << QString::fromStdString(str);
    return out;
}
