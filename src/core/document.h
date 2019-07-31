#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>

#include "smartPointers/sharedpointerdefs.h"
#include "singlewidgettarget.h"
#include "paintsettings.h"
#include "Paint/simplebrushwrapper.h"
#include "actions.h"
#include "Tasks/taskscheduler.h"
#include "clipboardcontainer.h"

class Gradient;
class FileDataCacheHandler;
class Canvas;
enum CanvasMode : short;

class Document : public SingleWidgetTarget {
    Q_OBJECT
    typedef stdsptr<FileDataCacheHandler> FileHandler;
    struct FileCompare {
        bool operator()(const FileHandler& f1, const FileHandler& f2);
    };
public:
    Document(TaskScheduler& taskScheduler);

    static Document* sInstance;

    Actions fActions;

    stdsptr<Clipboard> fClipboardContainer;

    QString fEvFile;

    bool fLocalPivot = true;
    CanvasMode fCanvasMode;

    // bookmarked
    QList<QColor> fColors;
    QList<SimpleBrushWrapper*> fBrushes;

    // all in document
    QList<qsptr<Gradient>> fGradients;

    QString fFontFamily;
    QString fFontStyle;
    qreal fFontSize;

    FillSettings fFill;
    StrokeSettings fStroke;

    QColor fBrushColor;
    const SimpleBrushWrapper* fBrush = nullptr;

    std::set<FileHandler, FileCompare> fFiles;

    QList<qsptr<Canvas>> fScenes;
    std::map<Canvas*, int> fVisibleScenes;
    Canvas* fActiveScene = nullptr;
    qptr<BoundingBox> fCurrentBox;

    void actionFinished();

    void replaceClipboard(const stdsptr<Clipboard>& container);
    DynamicPropsClipboard* getDynamicPropsClipboard() const;
    PropertyClipboard* getPropertyClipboard() const;
    KeysClipboard* getKeysClipboard() const;
    BoxesClipboard* getBoxesClipboard() const;

    void setPath(const QString& path) {
        fEvFile = path;
        emit evFilePathChanged(fEvFile);
    }

    void setCanvasMode(const CanvasMode mode);

    Canvas * createNewScene();
    bool removeScene(const qsptr<Canvas>& scene);
    bool removeScene(const int id);

    void addVisibleScene(Canvas * const scene);
    bool removeVisibleScene(Canvas * const scene);

    void setActiveScene(Canvas * const scene);
    void clearActiveScene();
    int getActiveSceneFrame() const;
    void setActiveSceneFrame(const int frame);
    void incActiveSceneFrame();
    void decActiveSceneFrame();

    Gradient * createNewGradient();
    Gradient * duplicateGradient(const int id);
    bool removeGradient(const qsptr<Gradient>& gradient);
    bool removeGradient(const int id);
    Gradient * getGradientWithRWId(const int rwId);

    void setBrush(const SimpleBrushWrapper * const brush) {
        fBrush = brush;
        if(fBrush) fBrush->setColor(fBrushColor);
    }

    void setBrushColor(const QColor &color) {
        fBrushColor = color;
        if(fBrush) fBrush->setColor(fBrushColor);
    }

    void incBrushRadius() {
        if(!fBrush) return;
        fBrush->incPaintBrushSize(0.3);
    }

    void decBrushRadius() {
        if(!fBrush) return;
        fBrush->decPaintBrushSize(0.3);
    }
//
    void clear();
//
    void write(QIODevice * const dst) const;
    void read(QIODevice * const src);

    void SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId);
private:
    Clipboard *getClipboard(const ClipboardType &type) const;

    void clearGradientRWIds() const;
    void writeGradients(QIODevice * const dst) const;
    void writeScenes(QIODevice * const dst) const;

    void readGradients(QIODevice * const src);
    void readScenes(QIODevice * const src);
signals:
    void canvasModeSet(CanvasMode);

    void sceneCreated(Canvas*);
    void sceneRemoved(Canvas*);
    void sceneRemoved(int);
//
    void activeSceneSet(Canvas*);

    void activeSceneFrameSet(int);
//
    void currentBoxChanged(BoundingBox*);
//
    void selectedPaintSettingsChanged();
//
    void gradientCreated(Gradient*);
    void gradientRemoved(Gradient*);
//
    void evFilePathChanged(QString);
    void documentChanged();
};

#endif // DOCUMENT_H
