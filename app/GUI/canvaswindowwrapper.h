#ifndef CANVASWINDOWWRAPPER_H
#define CANVASWINDOWWRAPPER_H
#include "stackwidgetwrapper.h"
class Document;
class Canvas;
class CanvasWindow;
class AudioHandler;

struct SceneWidgetStackLayoutItem : public WidgetStackLayoutItem {
    void clear();
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
    void setScene(Canvas* const scene);
protected:
    Canvas* mScene = nullptr;
};

struct CWWidgetStackLayoutItem : public SceneWidgetStackLayoutItem {
    CWWidgetStackLayoutItem(Document& document, AudioHandler& audioHandler) :
        mDocument(document), mAudioHandler(audioHandler) {}

    void clear();
    QWidget* create(QWidget * const parent);
    void write(QIODevice* const dst) const;
    void read(QIODevice* const src);
    void setTransform(const QMatrix& transform);
private:
    Document& mDocument;
    AudioHandler& mAudioHandler;
    QMatrix mTransform;
};

class CanvasWindowWrapper : public StackWidgetWrapper {
public:
    CanvasWindowWrapper(Document * const document,
                        AudioHandler * const audioHandler,
                        CWWidgetStackLayoutItem * const layItem,
                        QWidget * const parent = nullptr);

    void setScene(Canvas* const scene);
    Canvas* getScene() const;
    CanvasWindow *getSceneWidget() const;

    void saveDataToLayout() const;
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

#endif // CANVASWINDOWWRAPPER_H
