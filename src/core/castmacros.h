#ifndef CASTMACROS_H
#define CASTMACROS_H

#define rcConstChar(ptr) reinterpret_cast<const char*>(ptr)
#define rcChar(ptr) reinterpret_cast<char*>(ptr)
#define scChar(ptr) static_cast<char*>(ptr)
#define scCharConst(ptr) static_cast<const char*>(ptr)

#endif // CASTMACROS_H
