#include "nodesettings.h"
#include "pointhelpers.h"

NodeSettings::NodeSettings() : ctrlsMode(CtrlsMode::CTRLS_CORNER) {}

NodeSettings::NodeSettings(const NodeSettings *settings) {
    if(settings == nullptr) return;
    copyFrom(settings);
}

NodeSettings::NodeSettings(const bool &startEnabledT,
                           const bool &endEnabledT,
                           const CtrlsMode &ctrlsModeT) {
    set(startEnabledT, endEnabledT, ctrlsModeT);
}

void NodeSettings::copyFrom(const NodeSettings *settings) {
    if(settings == nullptr) {
        startEnabled = false;
        endEnabled = false;
        ctrlsMode = CtrlsMode::CTRLS_CORNER;
    } else {
        startEnabled = settings->startEnabled;
        endEnabled = settings->endEnabled;
        ctrlsMode = settings->ctrlsMode;
    }
}

void NodeSettings::set(const bool &startEnabledT,
                       const bool &endEnabledT,
                       const CtrlsMode &ctrlsModeT) {
    startEnabled = startEnabledT;
    endEnabled = endEnabledT;
    ctrlsMode = ctrlsModeT;
}
