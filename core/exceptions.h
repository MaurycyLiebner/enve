#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <exception>
#include <QDebug>
#define ERROUT(msg) qDebug() << msg << __LINE__
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define RuntimeThrow(msg) std::throw_with_nested(std::runtime_error(std::to_string(__LINE__) + "  :  " + FILENAME + "\n" + __func__ + "()\n" + msg))
#define CheckInvalidLocation(vLoc, name) \
    if(vLoc < 0) { \
        checkGlErrors(); \
        RuntimeThrow("Invalid " name " location."); \
    }

QDebug operator<<(QDebug out, const std::string& str);

extern void gPrintExceptionCritical(const std::exception& e,
                                    const QString& allText = "",
                                    const uint& level = 0);
extern void gPrintExceptionFatal(const std::exception& e,
                                 const QString &allText = "",
                                 const uint& level = 0);

#endif // EXCEPTIONS_H
