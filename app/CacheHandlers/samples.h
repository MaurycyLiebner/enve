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

    Samples(const Samples * const src) :
        Samples(src->fSampleRange) {
        const auto bytes = static_cast<ulong>(fSampleRange.span())*sizeof(float);
        memcpy(fData, src->fData, bytes);
    }

    Samples(const stdsptr<Samples>& src) :
        Samples(src.get()) {}
public:
    ~Samples() { delete[] fData; }
    float * const fData;
    const SampleRange fSampleRange;
};

#endif // SAMPLES_H
