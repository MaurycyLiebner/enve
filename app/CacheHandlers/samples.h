#ifndef SAMPLES_H
#define SAMPLES_H
#include "smartPointers/stdselfref.h"

struct Samples : public StdSelfRef {
    friend class StdSelfRef;
protected:
    Samples() : fData(nullptr), fSamplesCount(0) {}

    Samples(const int& size) :
        fData(static_cast<float*>(malloc(static_cast<size_t>(size)*sizeof(float)))),
        fSamplesCount(size) {}

    Samples(float * const data, const int& size) :
        fData(data), fSamplesCount(size) {}
public:
    ~Samples() { free(fData); }
    float * const fData;
    const int fSamplesCount;
};

#endif // SAMPLES_H
