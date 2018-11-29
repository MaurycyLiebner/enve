#ifndef BRUSHSETTINGSWIDGET_H
#define BRUSHSETTINGSWIDGET_H

#include <QWidget>
#include "Paint/brush.h"
#include "brushsettingwidget.h"
class ColorSettingsWidget;
class ColorSetting;
class ScrollArea;
class BrushSelectionScrollArea;

class BrushSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit BrushSettingsWidget(QWidget *parent = nullptr);
    void setBrushWidgetSetting(const BrushSetting &setting_id,
                               const qreal &val_t,
                               const bool &edited_t);
    void incBrushRadius();
    void decBrushRadius();
    void revertSettingToDefault(const BrushSetting &setting_t);
    bool hasSettingDefaultVal(const BrushSetting &setting_t);
    Brush *getCurrentBrush();
    void createNewBrush();

    void saveBrushesForProject(QIODevice *target);
    void readBrushesForProject(QIODevice *target);

    void setCurrentQColor(const QColor &color);
    QColor getCurrentQColor();
signals:
    void brushSelected(const Brush*);
    void brushReplaced(const Brush*, const Brush*); // second one is new
public slots:
    void setCurrentBrush(const Brush *brush);

    void setBrushSetting(const BrushSetting &setting_id,
                         const qreal &val_t);
    void showHideAdvancedSettings();
    void saveBrush();
    void setColorSetting(const ColorSetting &colorSetting);

private:
    bool mAdvancedSettingsVisible = false;
    BrushSelectionScrollArea *mBrushSelection = nullptr;
    ScrollArea *mAdvancedArea;
    QWidget *mAdvancedWidget;
    ColorSettingsWidget *mColorSettingsWidget;
    QHBoxLayout *mButtonsLayout = nullptr;
    QPushButton *save_brush_button = nullptr;
    QPushButton *mOverwriteBrushSettings = nullptr;
    QPushButton *mAdvancedButton = nullptr;

    Brush *mCurrentBrush = nullptr;
    QList<BrushSettingWidget*> mSettingWidgets;
    QVBoxLayout *mMainLayout = nullptr;
    QHBoxLayout *mHLayout = nullptr;
    QVBoxLayout *mLabelsLayout = nullptr;
    QVBoxLayout *mRestLayout = nullptr;

    QHBoxLayout *mAdvancedLayout;
    QVBoxLayout *mAdvancedLabelsLayout = nullptr;
    QVBoxLayout *mAdvancedRestLayout = nullptr;
};


#endif // BRUSHSETTINGSWIDGET_H
