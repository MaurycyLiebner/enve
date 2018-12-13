#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <exception>

#define DefException(eName, eWhat) struct eName : public std::exception { \
    const char* what() const noexcept { \
        return eWhat; \
    } \
};

#define MonoTry(tryF, eType) try { if(!tryF) throw eType(); } \
    catch(const eType& e) { qDebug() << e.what(); }

DefException(ContextCreateFailed, "Creating GL context failed.")
DefException(ContextCurrentFailed, "Making GL context current failed.")
DefException(InitializeGLFuncsFailed, "Initializing GL functions failed.")

#endif // EXCEPTIONS_H
