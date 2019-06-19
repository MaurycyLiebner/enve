#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>

#include "smartPointers/sharedpointerdefs.h"
#include "singlewidgettarget.h"
#include "paintsettings.h"

class Gradient;
class FileCacheHandler;
class Canvas;
enum CanvasMode : short;

class Document : public SingleWidgetTarget {
    Q_OBJECT
    typedef stdsptr<FileCacheHandler> FileHandler;
    struct FileCompare {
        bool operator()(const FileHandler& f1, const FileHandler& f2);
    };
public:
    Document() { sInstance = this; }

    static Document* sInstance;

    QString fEvFile;

    CanvasMode fCanvasMode;

    // bookmarked
    QList<QColor> fColors;
    QList<SimpleBrushWrapper*> fBrushes;

    // all in document
    QList<qsptr<Gradient>> fGradients;

    FillSettings fLastFill;
    StrokeSettings fLastStroke;

    QColor fLastBrushColor;
    SimpleBrushWrapper* fLastBrush = nullptr;

    std::set<FileHandler, FileCompare> fFiles;

    QList<qsptr<Canvas>> fScenes;
    qptr<Canvas> fTimelineScene;
    qptr<Canvas> fLastActiveScene;

    void setPath(const QString& path) {
        fEvFile = path;
        emit evFilePathChanged(fEvFile);
    }

    Canvas * createNewScene();
    bool removeScene(const qsptr<Canvas>& scene);
    bool removeScene(const int id);

    void setActiveScene(Canvas * const scene);

    void clear();

    void write(QIODevice * const dst) const;
    void read(QIODevice * const src);

    Gradient * getGradientWithRWId(const int rwId);

    void SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId);
private:
    void clearGradientRWIds() const;
    void writeGradients(QIODevice * const dst) const;
    void writeScenes(QIODevice * const dst) const;

    void readGradients(QIODevice * const src);
    void readScenes(QIODevice * const src);
signals:
    void sceneCreated(Canvas*);
    void sceneRemoved(Canvas*);
    void sceneRemoved(int);

    void activeSceneChanged(Canvas*);

    void evFilePathChanged(QString);
};

#endif // DOCUMENT_H
