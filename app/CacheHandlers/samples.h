#ifndef SAMPLES_H
#define SAMPLES_H
#include "smartPointers/stdselfref.h"
#include "framerange.h"

struct Samples : public StdSelfRef {
    friend class StdSelfRef;
protected:
    Samples(const SampleRange& range) :
        fData(new float[static_cast<size_t>(range.span())]),
        fSampleRange(range) {}

    Samples(float * const data, const SampleRange& range) :
        fData(data), fSampleRange(range) {}
public:
    ~Samples() { delete[] fData; }
    float * const fData;
    const SampleRange fSampleRange;
};

#endif // SAMPLES_H
