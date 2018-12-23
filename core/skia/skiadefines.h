#ifndef SKIADEFINES_H
#define SKIADEFINES_H
#include <QtGlobal>
//#define CPU_ONLY_RENDER

#ifdef QT_DEBUG
    #define GR_GL_CHECK_ERROR true
    #define GR_GL_LOG_CALLS true
    #define SK_DEBUG
    #undef SK_RELEASE
#else
    #define GR_GL_CHECK_ERROR false
    #define GR_GL_LOG_CALLS false
    #define SK_RELEASE
    #undef SK_DEBUG
#endif

#endif // SKIADEFINES_H
