#include "efiltersettings.h"

eFilterSettings* eFilterSettings::sInstance = nullptr;

eFilterSettings::eFilterSettings() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

void eFilterSettings::setEnveRenderFilter(const SkFilterQuality filter) {
    if(filter == mEnveRender) return;
    mEnveRender = filter;
    updateRenderFilter();
}

void eFilterSettings::setOutputRenderFilter(const SkFilterQuality filter) {
    if(filter == mOutputRender) return;
    mOutputRender = filter;
    updateRenderFilter();
}
