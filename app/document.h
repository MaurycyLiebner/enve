#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>

#include "smartPointers/sharedpointerdefs.h"
#include "singlewidgettarget.h"

class Gradient;
class FileCacheHandler;
class Canvas;

class Document : public SingleWidgetTarget {
    typedef stdsptr<FileCacheHandler> FileHandler;
    struct FileCompare {
        bool operator()(const FileHandler& f1, const FileHandler& f2);
    };
public:
    QString fEvFile;

    QList<QColor> fColors;
    QList<qsptr<Gradient>> fGradients;
    std::set<FileHandler, FileCompare> fFiles;
    QList<qsptr<Canvas>> fScenes;

    qptr<Canvas> fTimelineScene;

    void SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId);
};

#endif // DOCUMENT_H
