#ifndef EFILTERSETTINGS_H
#define EFILTERSETTINGS_H
#include <QObject>
#include "skia/skiaincludes.h"
#include "simplemath.h"

class eFilterSettings : public QObject {
    Q_OBJECT
public:
    eFilterSettings();

    static eFilterSettings* sInstance;

    static void sSetEnveRenderFilter(const SkFilterQuality filter) {
        sInstance->setEnveRenderFilter(filter);
    }

    static void sSetOutputRenderFilter(const SkFilterQuality filter) {
        sInstance->setOutputRenderFilter(filter);
    }

    static void sSetDisplayFilter(const SkFilterQuality filter) {
        sSetSmartDisplay(false);
        sInstance->mDisplayFilter = filter;
    }

    static void sSetSmartDisplay(const bool smart) {
        sInstance->mSmartDisplay = smart;;
    }

    static SkFilterQuality sRender() {
        return sInstance->mRender;
    }

    static SkFilterQuality sDisplay() {
        return sInstance->mDisplayFilter;
    }

    static bool sSmartDisplat() {
        return sInstance->mSmartDisplay;
    }

    static SkFilterQuality sDisplay(const qreal zoom,
                                    const qreal resolution) {
        if(sInstance->mSmartDisplay) {
            const qreal scale = zoom/resolution;
            if(isOne4Dec(scale)) return kNone_SkFilterQuality;
            else if(scale > 2.5) return kNone_SkFilterQuality;
            else if(scale < 0.5) return kMedium_SkFilterQuality;
            return kLow_SkFilterQuality;
        } else return sDisplay();
    }

    static void sSwitchToEnveRender() {
        sInstance->mCurrentRender = RenderFilter::enve;
        sInstance->updateRenderFilter();
    }

    static void sSwitchToOutputRender() {
        sInstance->mCurrentRender = RenderFilter::output;
        sInstance->updateRenderFilter();
    }

    void setEnveRenderFilter(const SkFilterQuality filter);
    void setOutputRenderFilter(const SkFilterQuality filter);
signals:
    void renderFilterChanged(const SkFilterQuality filter);
private:
    void updateRenderFilter() {
        SkFilterQuality newFilter;
        if(mCurrentRender == RenderFilter::enve) newFilter = mEnveRender;
        else newFilter = mOutputRender;
        if(newFilter == mRender) return;
        mRender = newFilter;
        emit renderFilterChanged(newFilter);
    }

    SkFilterQuality mEnveRender = SkFilterQuality::kHigh_SkFilterQuality;
    SkFilterQuality mOutputRender = SkFilterQuality::kHigh_SkFilterQuality;

    enum class RenderFilter { enve, output };
    RenderFilter mCurrentRender = RenderFilter::enve;
    SkFilterQuality mRender = mEnveRender;

    bool mSmartDisplay = true;
    SkFilterQuality mDisplayFilter = SkFilterQuality::kNone_SkFilterQuality;
};

#endif // EFILTERSETTINGS_H
