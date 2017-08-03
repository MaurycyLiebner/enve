#include "brushbutton.h"
#include <QPainter>
#include "brushselector.h"
#include "mainwindow.h"

BrushButton::BrushButton(QString icon_adress, QString plb_adress_t, BrushSelector *parent_t) : QGraphicsItem(parent_t)
{
    hide();
    brush_selector = parent_t;
    plb_adress = plb_adress_t;
    brush_icon.load(icon_adress);
    setAcceptHoverEvents(true);
}

void BrushButton::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
//    QColor col_t = MainWindow::getColor(darker, selected, hover).qcol;
//    p->fillRect(boundingRect(), col_t);
    int img_dim = brush_selector->getButtonImgDim();
    int button_dim = brush_selector->getButtonDim();
    int margin_t = (button_dim - img_dim)*0.5;
    p->drawPixmap(margin_t, margin_t, img_dim, img_dim, brush_icon);
}

QRectF BrushButton::boundingRect() const
{
    int dim_t = brush_selector->getButtonDim();
    return QRectF(0, 0, dim_t, dim_t);
}

void BrushButton::mousePressEvent(QGraphicsSceneMouseEvent *)
{

}

void BrushButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    select();
}

void BrushButton::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    hover = true;
    update();
}

void BrushButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    hover = false;
    update();
}

void BrushButton::select()
{
    setSelected(true);
}

void BrushButton::deselect()
{
    setSelected(false);
}

void BrushButton::setSelected(bool selected_t)
{
    if(selected_t == selected)
    {
        return;
    }
    if(selected_t)
    {
        brush_selector->selectBrush(this);
    }
    selected = selected_t;
    update();
}

void BrushButton::setDarker(bool darker_t)
{
    darker = darker_t;
    update();
}

bool BrushButton::isDarker()
{
    return darker;
}

QPixmap *BrushButton::getBrushIcon()
{
    return &brush_icon;
}

void BrushButton::setDefaultSettingsFromCurrent()
{
    default_settings = brush_settings;
    edited.clear();
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        edited.append(false);
    }
}

float BrushButton::getBrushSetting(BrushSetting setting_t)
{
    return brush_settings.at(setting_t);
}

struct BrushSettingFromString
{
    BrushSetting setting;
    float value;
    bool is_setting = true;
    BrushSettingFromString(QString setting_line_t)
    {
        for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++)
        {
            BrushSettingInfo setting_info = Brush::brush_settings_info[i];
            if(!setting_line_t.contains(":") )
            {
                continue;
            }
            QStringList line_split = setting_line_t.split(":");
            QString setting_name = line_split.first();
            setting_name = setting_name.trimmed();
            if(setting_name == setting_info.name )
            {
                setting = setting_info.setting;
                QString val_string = line_split.last();
                val_string = val_string.trimmed();
                value = val_string.toFloat();
                return;
            }
        }
        is_setting = false;
    }
};

void BrushButton::loadSettingsFromFileToList(QString plb_adress_t, QList<float> *settings_t)
{
    settings_t->clear();
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        settings_t->append(Brush::brush_settings_info[i].def);
    }
    QFile file(plb_adress_t);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream out(&file);
    while(!out.atEnd() )
    {
        BrushSettingFromString setting_from_string(out.readLine());
        if(setting_from_string.is_setting )
        {
            settings_t->replace(setting_from_string.setting, setting_from_string.value);
        }
    }
    file.close();
}

void BrushButton::loadBrushSettings()
{
    loadSettingsFromFileToList(plb_adress, &default_settings);
    setBrushSettingsFromDefault();
}

void BrushButton::setBrushSettingsFromDefault()
{
    brush_settings = default_settings;
    edited.clear();
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        edited.append(false);
    }
}

void BrushButton::setModifiedSettingsForBrush(Brush *brush_t)
{
    if(brush_settings.isEmpty())
    {
        loadBrushSettings();
    }
    brush_t->setBrushFilePath(plb_adress);
    brush_t->setBrushName(brush_name);
    brush_t->setCollectionName(collection_name);
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        brush_t->setSetting(static_cast<BrushSetting>(i),
                                     brush_settings.at(i));
    }
}
bool areSame(float a, float b)
{
    return qAbs(a - b) < 0.001f;
}

void BrushButton::setBrushSetting(BrushSetting setting_t, float val_t)
{
    brush_settings.replace(setting_t, val_t);
    edited.replace(setting_t, !areSame(val_t, default_settings.at(setting_t) ) );
}

float BrushButton::revertSettingToDefault(BrushSetting setting_t)
{
    float val_t = default_settings.at(setting_t);
    brush_settings.replace(setting_t, val_t);
    edited.replace(setting_t, false);
    return val_t;
}

bool BrushButton::getEdited(BrushSetting setting_t)
{
    return edited.at(setting_t);
}

void BrushButton::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    brush_selector->wheelEvent(e);
}

QString BrushButton::getBrushName()
{
    return brush_name;
}

QString BrushButton::getCollectionName()
{
    return collection_name;
}

void BrushButton::setBrushName(QString brush_name_t)
{
    brush_name = brush_name_t;
}

void BrushButton::setCollectionName(QString collection_name_t)
{
    collection_name = collection_name_t;
}
