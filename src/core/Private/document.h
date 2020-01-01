// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>

#include "smartPointers/ememory.h"
#include "singlewidgettarget.h"
#include "paintsettings.h"
#include "Paint/brushcontexedwrapper.h"
#include "actions.h"
#include "Tasks/taskscheduler.h"
#include "clipboardcontainer.h"

class Gradient;
class FileDataCacheHandler;
class Canvas;
enum class CanvasMode : short;

enum class NodeVisiblity {
    dissolvedAndNormal,
    dissolvedOnly,
    normalOnly
};

class Document : public SingleWidgetTarget {
    Q_OBJECT
    typedef stdsptr<FileDataCacheHandler> FileHandler;
    struct FileCompare {
        bool operator()(const FileHandler& f1, const FileHandler& f2);
    };
public:
    Document(TaskScheduler& taskScheduler);

    static Document* sInstance;

    stdsptr<Clipboard> fClipboardContainer;

    QString fEvFile;

    NodeVisiblity fNodeVisibility = NodeVisiblity::dissolvedAndNormal;
    bool fLocalPivot = true;
    CanvasMode fCanvasMode;

    // bookmarked
    QList<QColor> fColors;
    QList<SimpleBrushWrapper*> fBrushes;

    // all in document
    QList<qsptr<Gradient>> fGradients;

    Qt::Alignment fTextAlignment = Qt::AlignLeft;
    Qt::Alignment fTextVAlignment = Qt::AlignTop;
    QString fFontFamily = "Arial";
    QString fFontStyle = "Regular";
    qreal fFontSize = 72;

    FillSettings fFill;
    StrokeSettings fStroke;

    QColor fBrushColor;
    const SimpleBrushWrapper* fBrush = nullptr;
    bool fOnionVisible = true;

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
    SmartPathClipboard* getSmartPathClipboard() const;

    void setPath(const QString& path);

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
    Gradient * getGradientWithRWId(const int rwId) const;
    Gradient * getGradientWithDocumentId(const int id) const;

    void addBookmarkBrush(SimpleBrushWrapper* const brush);

    void removeBookmarkBrush(SimpleBrushWrapper* const brush);

    void addBookmarkColor(const QColor& color);

    void removeBookmarkColor(const QColor& color);

    void setBrush(BrushContexedWrapper * const brush);

    void setBrushColor(const QColor &color);

    void incBrushRadius();
    void decBrushRadius();

    void setOnionDisabled(const bool disabled);
//
    void clear();
//
    void write(eWriteStream &dst) const;
    void read(eReadStream &src);

    void SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId);
private:
    Clipboard *getClipboard(const ClipboardType type) const;

    void clearGradientRWIds() const;
    void writeBookmarked(eWriteStream &dst) const;
    void writeGradients(eWriteStream &dst) const;
    void writeScenes(eWriteStream &dst) const;

    void readBookmarked(eReadStream &src);
    void readGradients(eReadStream &src);
    void readScenes(eReadStream &src);
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
    void brushChanged(BrushContexedWrapper* brush);
    void brushColorChanged(QColor color);
    void brushSizeChanged(float size);
//
    void bookmarkColorAdded(QColor color);
    void bookmarkColorRemoved(QColor color);
    void bookmarkBrushAdded(SimpleBrushWrapper* brush);
    void bookmarkBrushRemoved(SimpleBrushWrapper* brush);
//
    void evFilePathChanged(QString);
    void documentChanged();
};

#endif // DOCUMENT_H
