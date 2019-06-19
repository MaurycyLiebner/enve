#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>

#include "smartPointers/sharedpointerdefs.h"
#include "singlewidgettarget.h"
#include "paintsettings.h"
#include "GUI/BrushWidgets/simplebrushwrapper.h"

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

    FillSettings fFill;
    StrokeSettings fStroke;

    QColor fBrushColor;
    const SimpleBrushWrapper* fBrush = nullptr;

    std::set<FileHandler, FileCompare> fFiles;

    QList<qsptr<Canvas>> fScenes;
    qptr<Canvas> fTimelineScene;
    qptr<Canvas> fActiveScene;

    void setPath(const QString& path) {
        fEvFile = path;
        emit evFilePathChanged(fEvFile);
    }

    Canvas * createNewScene();
    bool removeScene(const qsptr<Canvas>& scene);
    bool removeScene(const int id);

    void setActiveScene(Canvas * const scene);

    Gradient * createNewGradient();
    Gradient * duplicateGradient(const int id);
    bool removeGradient(const qsptr<Gradient>& gradient);
    bool removeGradient(const int id);

    void setBrush(const SimpleBrushWrapper * const brush) {
        fBrush = brush;
        if(fBrush) fBrush->setColor(fBrushColor);
    }

    void setBrushColor(const QColor &color) {
        fBrushColor = color;
        fBrushColor.setBlueF(color.redF());
        fBrushColor.setRedF(color.blueF());
        if(!fBrush) return;
        fBrush->setColor(fBrushColor);
    }

    void incBrushRadius() {
        if(!fBrush) return;
        fBrush->incPaintBrushSize(0.3);
    }

    void decBrushRadius() {
        if(!fBrush) return;
        fBrush->decPaintBrushSize(0.3);
    }

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

    void activeSceneSet(Canvas*);
    void activeSceneBoxSelectionChanged();

    void selectedPaintSettingsChanged();

    void gradientCreated(Gradient*);
    void gradientRemoved(Gradient*);
    void gradientRemoved(int);

    void evFilePathChanged(QString);
};

#endif // DOCUMENT_H
