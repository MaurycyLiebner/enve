#include "gpupostprocessor.h"

GpuPostProcessor::GpuPostProcessor() {
    glGenFramebuffers(1, &mFrameBufferId);
}
