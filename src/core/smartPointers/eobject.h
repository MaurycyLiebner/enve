#ifndef EOBJECT_H
#define EOBJECT_H

#define e_PROHIBIT_HEAP \
private: \
    static void *operator new (size_t sz) { \
        return std::malloc(sz); \
    }

#define e_OBJECT \
    friend class SelfRef; \
    friend class StdSelfRef; \
    e_PROHIBIT_HEAP

#endif // EOBJECT_H
