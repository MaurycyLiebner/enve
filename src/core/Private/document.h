// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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
#include <QDomDocument>

#include "smartPointers/ememory.h"
#include "singlewidgettarget.h"
#include "paintsettings.h"
#include "Paint/brushcontexedwrapper.h"
#include "actions.h"
#include "Tasks/taskscheduler.h"
#include "clipboardcontainer.h"
#include "Animators/SculptPath/sculptpath.h"
#include "conncontextptr.h"
#include "zipfilesaver.h"
#include "zipfileloader.h"

class SceneBoundGradient;
class FileDataCacheHandler;
class Canvas;
enum class CanvasMode : short;

enum class NodeVisiblity {
    dissolvedAndNormal,
    dissolvedOnly,
    normalOnly
};

enum class PaintMode {
    normal, erase, lockAlpha, colorize,
    move, crop
};

class CORE_EXPORT Document : public SingleWidgetTarget {
    Q_OBJECT
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

    Qt::Alignment fTextAlignment = Qt::AlignLeft;
    Qt::Alignment fTextVAlignment = Qt::AlignTop;
    QString fFontFamily = "Arial";
    SkFontStyle fFontStyle;
    qreal fFontSize = 72;

    FillSettings fFill;
    StrokeSettings fStroke;

    SimpleBrushWrapper* fOutlineBrush = nullptr;

    bool fSculptNodesVisible = true;
    SculptBrush fSculptBrush = SculptBrush(3, 0.5, 0.8, 1);
    SculptTarget fSculptTarget = SculptTarget::position;
    SculptMode fSculptMode = SculptMode::drag;

    bool fDrawPathManual = false;
    int fDrawPathSmooth = 25;
    qreal fDrawPathMaxError = 50;

    QColor fBrushColor;
    SimpleBrushWrapper* fBrush = nullptr;
    bool fOnionVisible = false;
    PaintMode fPaintMode = PaintMode::normal;

    QList<qsptr<Canvas>> fScenes;
    std::map<Canvas*, int> fVisibleScenes;
    ConnContextPtr<Canvas> fActiveScene;
    qptr<BoundingBox> fCurrentBox;

    void updateScenes();
    void actionFinished();

    void replaceClipboard(const stdsptr<Clipboard>& container);
    DynamicPropsClipboard* getDynamicPropsClipboard() const;
    PropertyClipboard* getPropertyClipboard() const;
    KeysClipboard* getKeysClipboard() const;
    BoxesClipboard* getBoxesClipboard() const;
    SmartPathClipboard* getSmartPathClipboard() const;

    void setPath(const QString& path);
    QString projectDirectory() const;

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

    void addBookmarkBrush(SimpleBrushWrapper* const brush);

    void removeBookmarkBrush(SimpleBrushWrapper* const brush);

    void addBookmarkColor(const QColor& color);

    void removeBookmarkColor(const QColor& color);
//
    void setBrush(BrushContexedWrapper * const brush);

    void setBrushColor(const QColor &color);

    void incBrushRadius();
    void decBrushRadius();

    void setOnionDisabled(const bool disabled);
    void setPaintMode(const PaintMode mode);
//
    void setSculptNodesHidden(const bool hidden);

    void incSculptBrushRadius();
    void decSculptBrushRadius();

    void incSculptHardness();
    void decSculptHardness();

    void incSculptOpacity();
    void decSculptOpacity();

    void setSculptTarget(const SculptTarget target);
    void setSculptMode(const SculptMode mode);
//
    void clear();
//
    void write(eWriteStream &dst) const;
    void read(eReadStream &src);

    void writeXEV(ZipFileSaver& fileSaver) const;
    void writeDoxumentXEV(QDomDocument& doc) const;
    void writeScenesXEV(ZipFileSaver& fileSaver) const;

    struct SceneSettingsXEV {
        QString fName;
        int fFrame;
        int fWidth;
        int fHeight;
        qreal fFps;
        bool fClip;
    };

    void readDoxumentXEV(const QDomDocument& doc,
                         QList<SceneSettingsXEV>& sceneSetts);
    void readScenesXEV(ZipFileLoader& fileLoader,
                       const QList<SceneSettingsXEV>& nameList);
    void readXEV(ZipFileLoader& fileLoader);

    void SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId);
private:
    Clipboard *getClipboard(const ClipboardType type) const;

    void writeBookmarked(eWriteStream &dst) const;
    void writeScenes(eWriteStream &dst) const;

    void readGradients(eReadStream& src);

    void readBookmarked(eReadStream &src);
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
    void brushChanged(BrushContexedWrapper* brush);
    void brushColorChanged(QColor color);
    void brushSizeChanged(float size);

    void paintModeChanged(const PaintMode mode);
//
    void bookmarkColorAdded(QColor color);
    void bookmarkColorRemoved(QColor color);
    void bookmarkBrushAdded(SimpleBrushWrapper* brush);
    void bookmarkBrushRemoved(SimpleBrushWrapper* brush);
//
    void evFilePathChanged(QString);
    void documentChanged();
//
    void sculptBrushSizeChanged(const qreal size);
    void sculptHardnessChanged(const qreal hardness);
    void sculptOpacityChanged(const qreal opacity);

    void sculptTargetChanged(const SculptTarget target);
    void sculptModeChanged(const SculptMode mode);
};

#endif // DOCUMENT_H
