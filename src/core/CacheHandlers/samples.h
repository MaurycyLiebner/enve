#ifndef SAMPLES_H
#define SAMPLES_H
#include "smartPointers/stdselfref.h"
#include "smartPointers/ememory.h"
#include "framerange.h"

struct Samples : public StdSelfRef {
    e_OBJECT
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

    stdsptr<Samples> mid(const SampleRange& range) const {
        if(!range.isValid()) RuntimeThrow("Invalid range");
        if(range.fMin < fSampleRange.fMin ||
           range.fMax > fSampleRange.fMax)
            RuntimeThrow("Range outside bounds");
        const auto data = new float[static_cast<size_t>(range.span())];

        memcpy(data, fData + (range.fMin - fSampleRange.fMin),
               static_cast<size_t>(range.span())*sizeof(float));

        return enve::make_shared<Samples>(data, range);
    }
};

#endif // SAMPLES_H
