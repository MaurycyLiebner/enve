#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <exception>
#include <QDebug>
#include "GL/gl.h"
#define ERROUT(msg) qDebug() << msg << __LINE__

#define DefException(eName, eWhat) struct eName : public std::exception { \
    const char* what() const noexcept { \
        return eWhat; \
    } \
};

#define MonoTry(tryF, eType) try { if(!tryF) throw eType(); } \
    catch(const eType& e) { ERROUT(e.what()); }

#define RuntimeThrow(msg) std::throw_with_nested(std::runtime_error(msg))
#define CheckInvalidLocation(vLoc, name) \
    if(vLoc < 0) { \
        checkGlErrors(); \
        RuntimeThrow("Invalid " name " location."); \
    }

DefException(ContextCreateFailed, "Creating GL context failed.");
DefException(ContextCurrentFailed, "Making GL context current failed.");
DefException(InitializeGLFuncsFailed, "Initializing GL functions failed.");

QDebug operator<<(QDebug out, const std::string& str);

extern void gPrintExceptionCritical(const std::exception& e,
                            QString allText = "",
                            const uint& level = 0);
extern void gPrintExceptionFatal(const std::exception& e,
                                    QString allText = "",
                                    const uint& level = 0);

struct ShaderCompilationFailed : public std::exception {
    ShaderCompilationFailed(GLchar (&infoLog)[1024],
                            const std::string& type) : mType(type) {
        memcpy(mInfoLog, infoLog, 1024*sizeof(GLchar));
    }
    const char* what() const noexcept {
        return ("ERROR::SHADER_COMPILATION_ERROR of type: " + mType + "\n" + mInfoLog).c_str();
    }
private:
    GLchar mInfoLog[1024];
    const std::string mType;
};

struct ProgramLinkingFailed : public std::exception {
    ProgramLinkingFailed(GLchar (&infoLog)[1024],
                         const std::string& type) : mType(type) {
        memcpy(mInfoLog, infoLog, 1024*sizeof(GLchar));
    }
    const char* what() const noexcept {
        return ("ERROR::PROGRAM_LINKING_ERROR of type: " + mType + "\n" + mInfoLog).c_str();
    }
private:
    GLchar mInfoLog[1024];
    const std::string mType;
};

#endif // EXCEPTIONS_H
