#ifndef BRUSHSELECTOR_H
#define BRUSHSELECTOR_H
#include <QGraphicsItem>
#include "brushbutton.h"
#include <QDirIterator>
#include <QString>
#include <QApplication>

struct WindowVariables;
#include <QDebug>
struct Collection
{
    QString folder_path;
    QString name;
    QList<BrushButton*> brushes;

    void ini(QString folder_path_t, BrushSelector *parent_t)
    {
        folder_path = folder_path_t;
        QStringList list_t = folder_path_t.split("/");
        name = list_t.takeLast();
        if(name.isEmpty())
        {
            name = list_t.takeLast();
        }

    // for each .plb file in directory create brush with .png icon
        QDirIterator it(folder_path, QStringList() << "*.plb",
                        QDir::Files, QDirIterator::Subdirectories);
        bool darker = false;
        while (it.hasNext())
        {
            QString brush_file_qstring = it.next();
            QString brush_file = brush_file_qstring;
            QString brush_icon_t = brush_file_qstring.replace(".plb", ".png");
            BrushButton *brush_t = new BrushButton(brush_icon_t, brush_file, parent_t);
            brush_t->setCollectionName(name);

            QString brush_name_t;
            QFile file(brush_file);
            file.open(QFile::ReadOnly | QFile::Text);
            QTextStream out(&file);
            while(!out.atEnd() )
            {
                QString line_t = out.readLine();
                if(line_t.contains("name:"))
                {
                    brush_name_t = line_t.split(":").last();
                }
            }
            file.close();

            brush_t->setBrushName(brush_name_t);
            brush_t->setDarker(darker);
            brushes.append(brush_t);
            darker = !darker;
        }
    }
};

class BrushSelector : public QGraphicsItem
{
public:
    BrushSelector(WindowVariables *window_vars_t);

    void setBrushCollection(Collection *current_collection_t);
    void loadCollections(QString from_folder_t);
    void updateDisplayedBrushes();
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    void setWidth(int width_t);
    void setBrushButtonDimensions(int button_img_dim_t, int button_border_t);
    void selectBrush(BrushButton *button_t);
    void clearCurrentBrushButtons();
    int getButtonDim();
    int getButtonImgDim();
    int getButtonBorderW();
    void wheelEvent(QGraphicsSceneWheelEvent *e);
private:
    WindowVariables *window_vars;
    BrushButton *current_button = NULL;
    int button_img_dim = 0;
    int button_dim = 0;
    int min_border = 0;
    int n_columns = 0;
    int n_brushes = 0;
    int width = 0;
    int height = 0;
    Collection *current_collection = NULL;
};

#endif // BRUSHSELECTOR_H
