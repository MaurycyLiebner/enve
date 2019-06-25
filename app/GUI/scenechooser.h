#ifndef SCENECHOOSER_H
#define SCENECHOOSER_H
#include <QMenu>
class Canvas;
class Document;

class SceneChooser : public QMenu {
    Q_OBJECT
public:
    SceneChooser(Document &document, QWidget * const parent = nullptr);

    void setCurrentScene(Canvas * const scene);
    Canvas* getCurrentScene() const { return mCurrentScene; }
signals:
    void currentChanged(Canvas*);
private:
    void addScene(Canvas * const scene);
    void removeScene(Canvas * const scene);

    void setCurrentScene(Canvas * const scene, QAction * const act);

    Document& mDocument;
    Canvas * mCurrentScene = nullptr;
    std::map<Canvas*, QAction*> mSceneToAct;
};

#endif // SCENECHOOSER_H
