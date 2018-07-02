#include "brushsettingswidget.h"
#include "Colors/helpers.h"
#include <QPushButton>
#include "Colors/ColorWidgets/colorsettingswidget.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "brushselectionscrollarea.h"
#include "global.h"

BrushSettingsWidget::BrushSettingsWidget(QWidget *parent)
    : QWidget(parent) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    mMainLayout = new QVBoxLayout(this);

    mColorSettingsWidget = new ColorSettingsWidget(this);
    mColorSettingsWidget->hideAlphaControlers();
    connect(mColorSettingsWidget, SIGNAL(colorSettingSignal(ColorSetting)),
            this, SLOT(setColorSetting(ColorSetting)));
    mMainLayout->addWidget(mColorSettingsWidget);

    mBrushSelection = new BrushSelectionScrollArea(this);
    connect(mBrushSelection, SIGNAL(brushSelected(const Brush*)),
            this, SIGNAL(brushSelected(const Brush*)));
    connect(mBrushSelection, SIGNAL(brushReplaced(const Brush*, const Brush*)),
            this, SIGNAL(brushReplaced(const Brush*, const Brush*)));
    mMainLayout->addWidget(mBrushSelection);
    connect(mBrushSelection, SIGNAL(brushSelected(const Brush*)),
            this, SLOT(setCurrentBrush(const Brush*)));

    h_layout = new QHBoxLayout();
    mMainLayout->addLayout(h_layout);
    labels_layout = new QVBoxLayout();
    rest_layout = new QVBoxLayout();

    mAdvancedArea = new ScrollArea(this);
    mAdvancedArea->setMinimumHeight(10*MIN_WIDGET_HEIGHT);
    mAdvancedWidget = new QWidget(this);
    mAdvancedArea->setWidget(mAdvancedWidget);

    mAdvancedLayout = new QHBoxLayout();
    mAdvancedLabelsLayout = new QVBoxLayout();
    mAdvancedRestLayout = new QVBoxLayout();

    mAdvancedLayout->addLayout(mAdvancedLabelsLayout);
    mAdvancedLayout->addLayout(mAdvancedRestLayout);
    mAdvancedWidget->setLayout(mAdvancedLayout);
    mMainLayout->addWidget(mAdvancedArea);

    h_layout->addLayout(labels_layout);
    h_layout->addLayout(rest_layout);
    for(int i = 0; i < BRUSH_SETTING_ASPECT_RATIO; i++) {
        BrushSettingWidget *setting_widget_t =
                new BrushSettingWidget(labels_layout,
                                       rest_layout,
                                       static_cast<BrushSetting>(i),
                                       this);
        setting_widgets.append(setting_widget_t);
        connect(setting_widget_t, SIGNAL(setBrushSetting(BrushSetting,qreal)),
                this, SLOT(setBrushSetting(BrushSetting,qreal)));
    }
    for(int i = BRUSH_SETTING_ASPECT_RATIO; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSettingWidget *setting_widget_t =
                new BrushSettingWidget(mAdvancedLabelsLayout,
                                       mAdvancedRestLayout,
                                       static_cast<BrushSetting>(i),
                                       this);
        setting_widgets.append(setting_widget_t);
        connect(setting_widget_t, SIGNAL(setBrushSetting(BrushSetting,qreal)),
                this, SLOT(setBrushSetting(BrushSetting,qreal)));
    }
    mAdvancedArea->hide();
    //setting_widgets.at(BRUSH_SETTING_SLOW_TRACKING)->show();

    buttons_layout = new QHBoxLayout();
    save_brush_button = new QPushButton("Save Brush", this);
    connect(save_brush_button, SIGNAL(released()),
            this, SLOT(saveBrush()) );
    buttons_layout->addWidget(save_brush_button);

    advanced_button = new QPushButton("Advanced Settings", this);
    connect(advanced_button, SIGNAL(released()),
            this, SLOT(showHideAdvancedSettings()) );
    buttons_layout->addWidget(advanced_button);
    mMainLayout->addLayout(buttons_layout);

    setLayout(mMainLayout);
    setCurrentBrush(mBrushSelection->getCurrentBrush());
}

void BrushSettingsWidget::setBrushWidgetSetting(const BrushSetting &setting_id,
                                                const qreal &val_t,
                                                const bool &edited_t) {
    setting_widgets.at(setting_id)->setVal(val_t, edited_t);
}

void BrushSettingsWidget::setCurrentBrush(const Brush *brushT) {
    mCurrentBrush = const_cast<Brush*>(brushT);
    if(mCurrentBrush == nullptr) return;
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSetting id_t = static_cast<BrushSetting>(i);
        setBrushWidgetSetting(id_t,
                        brushT->getSettingVal(id_t),
                        false);
    }
    mBrushSelection->setCurrentBrush(mCurrentBrush);
}

void BrushSettingsWidget::setBrushSetting(
        const BrushSetting &setting_id,
        const qreal &val_t) {
    mCurrentBrush->setSetting(setting_id, val_t);
}

void BrushSettingsWidget::revertSettingToDefault(
        const BrushSetting &setting_t) {
    setBrushWidgetSetting(setting_t,
                    mCurrentBrush->getBrushSettingInfo(setting_t)->def,
                    false);
}

bool BrushSettingsWidget::hasSettingDefaultVal(
        const BrushSetting &setting_t) {
    return true;
}

Brush *BrushSettingsWidget::getCurrentBrush() {
    return mCurrentBrush;
}

void BrushSettingsWidget::saveBrushesForProject(QIODevice *target) {
    mBrushSelection->saveBrushesForProject(target);
}

void BrushSettingsWidget::readBrushesForProject(QIODevice *target) {
    mBrushSelection->readBrushesForProject(target);
}

void BrushSettingsWidget::setCurrentQColor(const QColor &color) {
    mColorSettingsWidget->setCurrentColor(color);
}

QColor BrushSettingsWidget::getCurrentQColor() {
    return mColorSettingsWidget->getCurrentQColor();
}

void BrushSettingsWidget::showHideAdvancedSettings() {
    advanced_settings_visible = !advanced_settings_visible;
    mAdvancedArea->setVisible(advanced_settings_visible);
}
#include <QFileDialog>
void BrushSettingsWidget::saveBrush() {
    //new BrushSaveDialog(window_vars);
    mCurrentBrush->setProjectOnly(false);
    QString collectionT = mCurrentBrush->getCollectionName();
    if(!collectionT.isEmpty()) collectionT += "/";
    QString dir = QDir::homePath() + "/.AniVect/";
    QString saveAs = dir +
            collectionT +
            mCurrentBrush->getBrushName() + ".avb";
    if(QDir::home().mkpath(dir)) {
        QFile file(saveAs);
        file.open(QFile::WriteOnly);
        file.resize(0);
        mCurrentBrush->writeBrush(&file);
        file.close();
        mBrushSelection->update();
    }
}

void BrushSettingsWidget::setColorSetting(const ColorSetting &colorSetting) {
    ColorMode colorMode = colorSetting.getSettingMode();
    if(colorMode == ColorMode::HSVMODE) {
        Brush::setHSV(colorSetting.getVal1(),
                      colorSetting.getVal2(),
                      colorSetting.getVal3());
    } else if(colorMode == ColorMode::HSLMODE) {
        Color color;
        color.setHSL(colorSetting.getVal1(),
                     colorSetting.getVal2(),
                     colorSetting.getVal3(),
                     colorSetting.getAlpa());
        Brush::setHSV(color.gl_h,
                      color.gl_s,
                      color.gl_v);
    } else if(colorMode == ColorMode::RGBMODE) {
        Color color;
        color.setRGB(colorSetting.getVal1(),
                     colorSetting.getVal2(),
                     colorSetting.getVal3(),
                     colorSetting.getAlpa());
        Brush::setHSV(color.gl_h,
                      color.gl_s,
                      color.gl_v);
    }
}

float BrushSettingsWidget::getBrushSetting(const BrushSetting &settind_id) {
    return setting_widgets.at(settind_id)->getVal();
}

void BrushSettingsWidget::incBrushRadius() {
    BrushSettingWidget *setting_widget_t =
            setting_widgets.at(BRUSH_SETTING_RADIUS);
    setting_widget_t->incVal(mCurrentBrush->getRadius()*0.2 + 0.3);
}

void BrushSettingsWidget::decBrushRadius() {
    BrushSettingWidget *setting_widget_t =
            setting_widgets.at(BRUSH_SETTING_RADIUS);
    setting_widget_t->incVal(-(mCurrentBrush->getRadius()*0.2 + 0.3) );
}
