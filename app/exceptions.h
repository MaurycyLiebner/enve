#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <exception>
#include <QDebug>
#define ERROUT(msg) qDebug() << msg << __LINE__

#define DefException(eName, eWhat) struct eName : public std::exception { \
    const char* what() const noexcept { \
        return eWhat; \
    } \
};

#define MonoTry(tryF, eType) try { if(!tryF) throw eType(); } \
    catch(const eType& e) { ERROUT(e.what()); }

DefException(ContextCreateFailed, "Creating GL context failed.")
DefException(ContextCurrentFailed, "Making GL context current failed.")
DefException(InitializeGLFuncsFailed, "Initializing GL functions failed.")
QDebug operator<<(QDebug out, const std::string& str);

#endif // EXCEPTIONS_H
