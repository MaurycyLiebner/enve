#include "nodesettings.h"
#include "pointhelpers.h"

NodeSettings::NodeSettings() : fCtrlsMode(CtrlsMode::CTRLS_CORNER) {}

NodeSettings::NodeSettings(const NodeSettings *settings) {
    if(!settings) return;
    copyFrom(settings);
}

NodeSettings::NodeSettings(const bool &startEnabledT,
                           const bool &endEnabledT,
                           const CtrlsMode &ctrlsModeT) {
    set(startEnabledT, endEnabledT, ctrlsModeT);
}

void NodeSettings::copyFrom(const NodeSettings *settings) {
    if(!settings) {
        fStartEnabled = false;
        fEndEnabled = false;
        fCtrlsMode = CtrlsMode::CTRLS_CORNER;
    } else {
        fStartEnabled = settings->fStartEnabled;
        fEndEnabled = settings->fEndEnabled;
        fCtrlsMode = settings->fCtrlsMode;
    }
}

void NodeSettings::set(const bool &startEnabledT,
                       const bool &endEnabledT,
                       const CtrlsMode &ctrlsModeT) {
    fStartEnabled = startEnabledT;
    fEndEnabled = endEnabledT;
    fCtrlsMode = ctrlsModeT;
}
