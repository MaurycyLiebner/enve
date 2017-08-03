#ifndef BRUSHBUTTON_H
#define BRUSHBUTTON_H
#include <QGraphicsItem>
#include <GL/gl.h>
#include "../PaintLib/brush.h"
class BrushSelector;

struct ButtonGLTex;

class BrushButton : public QGraphicsItem
{
public:
    BrushButton(QString icon_adress, QString plb_adress_t, BrushSelector *parent_t = NULL);
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    void select();
    void deselect();
    void setSelected(bool selected_t);
    void setDarker(bool darker_t);
    bool isDarker();
    QPixmap *getBrushIcon();

    void setDefaultSettingsFromCurrent();
    float getBrushSetting(BrushSetting setting_t);
    void loadBrushSettings();
    void setBrushSettingsFromDefault();
    void setModifiedSettingsForBrush(Brush *brush_t);
    void setBrushSetting(BrushSetting setting_t, float val_t);
    float revertSettingToDefault(BrushSetting setting_t);
    bool getEdited(BrushSetting setting_t);
    bool compareSettingToDefault(BrushSetting setting_t, float val_t);
    void wheelEvent(QGraphicsSceneWheelEvent *e);

    QString getBrushName();
    QString getCollectionName();

    void setBrushName(QString brush_name_t);
    void setCollectionName(QString collection_name_t);
    static void loadSettingsFromFileToList(QString plb_adress_t, QList<float> *settings_t);
private:
    QString brush_name;
    QString collection_name;

    QList<float> default_settings;
    QList<float> brush_settings;
    QList<bool> edited;
    bool darker = false;
    bool hover = false;
    BrushSelector *brush_selector = NULL;
    bool selected = false;
    QPixmap brush_icon;
    QString plb_adress;
};

#endif // BRUSHBUTTON_H
