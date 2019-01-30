#ifndef AUDIODECODE_H
#define AUDIODECODE_H
#include "framerange.h"

int gDecodeSoundDataRange(const char* path,
                          const SampleRange &range,
                          float *&audioData);

int gDecodeAudioFile(const char* path,
                     const int sample_rate,
                     float** audioData,
                     int* size);

#endif // AUDIODECODE_H
