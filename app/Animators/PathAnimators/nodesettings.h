#ifndef NODESETTINGS_H
#define NODESETTINGS_H
#include "smartPointers/stdselfref.h"
enum CtrlsMode : short;
struct NodeSettings : public StdSelfRef {
    friend class StdSelfRef;
    NodeSettings();
    NodeSettings(const NodeSettings *settings);
    NodeSettings(const bool &startEnabledT,
                 const bool &endEnabledT,
                 const CtrlsMode &ctrlsModeT);

    void copyFrom(const NodeSettings *settings);

    void set(const bool &startEnabledT,
             const bool &endEnabledT,
             const CtrlsMode &ctrlsModeT);

    bool startEnabled = false;
    bool endEnabled = false;
    CtrlsMode ctrlsMode;

    void write(QIODevice *target);
    void read(QIODevice *target);
};

#endif // NODESETTINGS_H
