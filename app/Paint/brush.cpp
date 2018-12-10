#include "brush.h"
#include "GUI/ColorWidgets/helpers.h"
qreal Brush::h = 0.;
qreal Brush::s = 0.;
qreal Brush::v = 0.;

qreal Brush::red = 0.;
qreal Brush::green = 0.;
qreal Brush::blue = 0.;

BrushSettingInfo Brush::brush_settings_info[BRUSH_SETTINGS_COUNT] = {
        BrushSettingInfo(BRUSH_SETTING_RADIUS, 1.f, 100.f, 5.f, "radius"),
        BrushSettingInfo(BRUSH_SETTING_OPACITY, 0.f, 1.f, 0.5f, "opacity"),
        BrushSettingInfo(BRUSH_SETTING_HARDNESS, 0.f, 1.f, 0.5f, "hardness"),
        BrushSettingInfo(BRUSH_SETTING_ASPECT_RATIO, 1.f, 10.f, 1.f, "aspect ratio"),
        BrushSettingInfo(BRUSH_SETTING_DISTANCE_BETWEEN_DABS, 0.01f, 2.f, 0.25f, "distance between dabs"),
        BrushSettingInfo(BRUSH_SETTING_INITIAL_ROTATION, 0.f, 180.f, 0.f, "initial rotation"),
        BrushSettingInfo(BRUSH_SETTING_ROTATION_INFLUENCE, 0.f, 1.f, 0.f, "rotation influence"),
        BrushSettingInfo(BRUSH_SETTING_ROTATION_DELAY, 0.f, 1.f, 0.f, "rotation delay"),
        BrushSettingInfo(BRUSH_SETTING_ROTATION_BETWEEN_DABS, 0.f, 90.f, 0.f, "rotation between dabs"),
        BrushSettingInfo(BRUSH_SETTING_CONTINUES_ALPHA, 0.f, 1.f, 0.5f, "continues alpha"),
        BrushSettingInfo(BRUSH_SETTING_INITIAL_ALPHA, 0.f, 10.f, 0.5f, "initial alpha"),
        BrushSettingInfo(BRUSH_SETTING_COLOR_PICK_UP, 0.f, 1.f, 0.5f, "color pick up"),
        BrushSettingInfo(BRUSH_SETTING_COLOR_DECAY, 0.f, 1.f, 0.5f, "color decay"),
        BrushSettingInfo(BRUSH_SETTING_DAB_POSITION_NOISE, 0.f, 5.f, 0.f, "dab position noise"),
        BrushSettingInfo(BRUSH_SETTING_STROKE_POSITION_NOISE, 0.f, 100.f, 0.f, "stroke position noise"),
        BrushSettingInfo(BRUSH_SETTING_STROKE_POSITION_NOISE_FREQUENCY, 0.f, 1.f, 0.f, "stroke position noise frequency"),

        BrushSettingInfo(BRUSH_SETTING_ROTATION_NOISE, 0.f, 90.f, 0.f, "rotation noise"),
        BrushSettingInfo(BRUSH_SETTING_ROTATION_NOISE_FREQUENCY, 0.f, 1.f, 0.f, "rotation noise frequency"),

        BrushSettingInfo(BRUSH_SETTING_HUE_NOISE, 0.f, 1.f, 0.f, "hue noise"),
        BrushSettingInfo(BRUSH_SETTING_HUE_NOISE_FREQUENCY, 0.f, 1.f, 0.f, "hue noise frequency"),

        BrushSettingInfo(BRUSH_SETTING_SATURATION_NOISE, 0.f, 1.f, 0.f, "saturation noise"),
        BrushSettingInfo(BRUSH_SETTING_SATURATION_NOISE_FREQUENCY, 0.f, 1.f, 0.f, "saturation noise frequency"),

        BrushSettingInfo(BRUSH_SETTING_VALUE_NOISE, 0.f, 1.f, 0.f, "value noise"),
        BrushSettingInfo(BRUSH_SETTING_VALUE_NOISE_FREQUENCY, 0.f, 1.f, 0.f, "value noise frequency"),

        BrushSettingInfo(BRUSH_SETTING_PRESSURE_RADIUS_GAIN, -1.f, 2.f, 0.f, "pressure radius gain"),
        BrushSettingInfo(BRUSH_SETTING_PRESSURE_OPACITY_GAIN, 0.f, 1.f, 0.f, "pressure opacity gain"),
        BrushSettingInfo(BRUSH_SETTING_PRESSURE_HARDNESS_GAIN, 0.f, 1.f, 0.f, "pressure hardness gain"),
        BrushSettingInfo(BRUSH_SETTING_PRESSURE_ALPHA_GAIN, 0.f, 1.f, 0.f, "pressure alpha gain"),
        BrushSettingInfo(BRUSH_SETTING_PRESSURE_ASCPECT_RATIO_GAIN, -1.f, 1.f, 0.f, "pressure aspect ratio gain"),

        BrushSettingInfo(BRUSH_SETTING_SPEED_RADIUS_GAIN, -1.f, 2.f, 0.f, "speed radius gain"),
        BrushSettingInfo(BRUSH_SETTING_SPEED_OPACITY_GAIN, -1.f, 1.f, 0.f, "speed opacity gain"),
        BrushSettingInfo(BRUSH_SETTING_SPEED_HARDNESS_GAIN, -1.f, 1.f, 0.f, "speed hardness gain")
    };

Brush::Brush() {
    loadDefaultSettings();
}

qreal *Brush::getSettingValuePointer(const BrushSetting &setting_t) {
    return const_cast<qreal*>(getSettingValuePointerConst(setting_t));
}

const qreal *Brush::getSettingValuePointerConst(const BrushSetting &setting_t) const {
    if(setting_t == BRUSH_SETTING_RADIUS) {
        return &radius;
    } else if(setting_t == BRUSH_SETTING_OPACITY) {
        return &opacity;
    } else if(setting_t == BRUSH_SETTING_HARDNESS) {
        return &hardness;
    } else if(setting_t == BRUSH_SETTING_ASPECT_RATIO) {
        return &aspect_ratio;
    } else if(setting_t == BRUSH_SETTING_DISTANCE_BETWEEN_DABS) {
        return &distance_between_dabs;
    } else if(setting_t == BRUSH_SETTING_INITIAL_ROTATION) {
        return &initial_rotation;
    } else if(setting_t == BRUSH_SETTING_ROTATION_INFLUENCE) {
        return &rotation_influence;
    } else if(setting_t == BRUSH_SETTING_ROTATION_DELAY) {
        return &rotation_delay;
    } else if(setting_t == BRUSH_SETTING_ROTATION_BETWEEN_DABS) {
        return &rotation_between_dabs;
    } else if(setting_t == BRUSH_SETTING_CONTINUES_ALPHA) {
        return &continuesAlpha;
    } else if(setting_t == BRUSH_SETTING_INITIAL_ALPHA) {
        return &initial_alpha;
    } else if(setting_t == BRUSH_SETTING_COLOR_PICK_UP) {
        return &color_pick_up;
    } else if(setting_t == BRUSH_SETTING_COLOR_DECAY) {
        return &color_decay;
    } else if(setting_t == BRUSH_SETTING_DAB_POSITION_NOISE) {
        return &dab_position_noise;
    } else if(setting_t == BRUSH_SETTING_STROKE_POSITION_NOISE) {
        return &stroke_position_noise;
    } else if(setting_t == BRUSH_SETTING_STROKE_POSITION_NOISE_FREQUENCY) {
        return &stroke_position_noise_frequency;
    } else if(setting_t == BRUSH_SETTING_ROTATION_NOISE) {
        return &rotation_noise;
    } else if(setting_t == BRUSH_SETTING_ROTATION_NOISE_FREQUENCY) {
        return &rotation_noise_frequency;
    } else if(setting_t == BRUSH_SETTING_HUE_NOISE) {
        return &hue_noise;
    } else if(setting_t == BRUSH_SETTING_HUE_NOISE_FREQUENCY) {
        return &hue_noise_frequency;
    } else if(setting_t == BRUSH_SETTING_SATURATION_NOISE) {
        return &saturation_noise;
    } else if(setting_t == BRUSH_SETTING_SATURATION_NOISE_FREQUENCY) {
        return &saturation_noise_frequency;
    } else if(setting_t == BRUSH_SETTING_VALUE_NOISE) {
        return &value_noise;
    } else if(setting_t == BRUSH_SETTING_VALUE_NOISE_FREQUENCY) {
        return &value_noise_frequency;
    } else if(setting_t == BRUSH_SETTING_PRESSURE_RADIUS_GAIN) {
        return &pressure_radius_gain;
    } else if(setting_t == BRUSH_SETTING_PRESSURE_OPACITY_GAIN) {
        return &pressure_opacity_gain;
    } else if(setting_t == BRUSH_SETTING_PRESSURE_HARDNESS_GAIN) {
        return &pressure_hardness_gain;
    } else if(setting_t == BRUSH_SETTING_PRESSURE_ALPHA_GAIN) {
        return &pressure_alpha_gain;
    } else if(setting_t == BRUSH_SETTING_PRESSURE_ASCPECT_RATIO_GAIN) {
        return &pressure_aspect_ratio_gain;
    } else if(setting_t == BRUSH_SETTING_SPEED_RADIUS_GAIN) {
        return &speed_radius_gain;
    } else if(setting_t == BRUSH_SETTING_SPEED_HARDNESS_GAIN) {
        return &speed_hardness_gain;
    } else if(setting_t == BRUSH_SETTING_SPEED_OPACITY_GAIN) {
        return &speed_opacity_gain;
    }
    return nullptr;
}

qreal Brush::getDistBetweenDabs() const {
    return distance_between_dabs;
}

qreal Brush::getDistBetweenDabsPx() const {
    return distance_between_dabs*radius;
}

void Brush::setHSV(qreal h_t, qreal s_t, qreal v_t) {
    h = h_t;
    s = s_t;
    v = v_t;
    red = h;
    green = s;
    blue = v;
    qhsv_to_rgb(red, green, blue);
}

void Brush::setSetting(BrushSetting setting_t, qreal val_t) {
    *getSettingValuePointer(setting_t) = val_t;
}

void Brush::loadDefaultSettings() {
    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSettingInfo setting_info = brush_settings_info[i];
        setSetting(setting_info.setting, setting_info.def);
    }
}

QString Brush::getSettingAsFileLine(BrushSetting setting_t) const {
    const BrushSettingInfo *setting_info_t = getBrushSettingInfo(setting_t);
    return setting_info_t->name + ":" + QString::number(getSettingVal(setting_t), 2, 3 );
}

void Brush::setBrushName(QString brush_name_t) {
    brush_name = brush_name_t;
}

void Brush::setCollectionName(QString collection_name_t) {
    collection_name = collection_name_t;
}

void Brush::setBrushFilePath(QString path_t) {
    brush_file_path = path_t;
}

QString Brush::getBrushName() const {
    return brush_name;
}

QString Brush::getCollectionName() const {
    return collection_name;
}

QString Brush::getBrushFilePath() const {
    return brush_file_path;
}

qreal Brush::getSettingVal(BrushSetting setting_t) const {
    return *getSettingValuePointerConst(setting_t);
}

const BrushSettingInfo *Brush::getBrushSettingInfo(BrushSetting setting_t) {
    return &brush_settings_info[setting_t];
}

void Brush::getRGB(qreal *red_t,
                   qreal *green_t,
                   qreal *blue_t) {
    *red_t = red;
    *green_t = green;
    *blue_t = blue;
}

void Brush::getHSV(qreal *h_t,
                   qreal *s_t,
                   qreal *v_t) {
    *h_t = h;
    *s_t = s;
    *v_t = v;
}

qreal Brush::getHardness() const {
    return hardness;
}

qreal Brush::getOpacity() const {
    return opacity;
}

qreal Brush::getRadius() const {
    return radius;
}

qreal Brush::getAspectRatio() const {
    return aspect_ratio;
}

qreal Brush::getColorPickUp() const {
    return color_pick_up;
}

qreal Brush::getColorPickUpDecay() const {
    return color_decay;
}

qreal Brush::getInitialRotation() const {
    return initial_rotation;
}

qreal Brush::getRotationInfluence() const {
    return rotation_influence;
}

qreal Brush::getRotationDelay() const {
    return rotation_delay;
}

qreal Brush::getRotationNoise() const {
    return rotation_noise;
}

qreal Brush::getRotationNoiseFrequency() const {
    return rotation_noise_frequency;
}

qreal Brush::getDabPositionNoisePx() const {
    return dab_position_noise*radius;
}

qreal Brush::getStrokePositionNoisePx() const {
    return stroke_position_noise*radius;
}

qreal Brush::getStrokePositionNoiseFrequency() const {
    return stroke_position_noise_frequency;
}

qreal Brush::getHueNoise() const {
    return hue_noise;
}

qreal Brush::getHueNoiseFrequency() const {
    return hue_noise_frequency;
}

qreal Brush::getValueNoise() const {
    return value_noise;
}

qreal Brush::getValueNoiseFrequency() const {
    return value_noise_frequency;
}

qreal Brush::getSaturationNoise() const {
    return saturation_noise;
}

qreal Brush::getSaturationNoiseFrequency() const {
    return saturation_noise_frequency;
}

qreal Brush::getRotationBetweenDabs() const {
    return rotation_between_dabs;
}

qreal Brush::getPressureRadiusGain() const {
    return pressure_radius_gain;
}

void Brush::setStrokeBrushFilePath(QString path_t) {
    stroke_brush_file_path = path_t;
}

QString Brush::getStrokeBrushFilePath() const {
    return stroke_brush_file_path;
}

bool Brush::wasChangedSinceLastStroke() const {
    return changed_since_last_stroke;
}

void Brush::setChangedSinceLastStroke(bool b_t) {
    changed_since_last_stroke = b_t;
}

qreal Brush::getPressureRadiusGainPx() const {
    return pressure_radius_gain*radius;
}

qreal Brush::getPressureOpacityGain() const {
    return pressure_opacity_gain;
}

qreal Brush::getPressureHardnessGain() const {
    return pressure_hardness_gain;
}

qreal Brush::getPressureAlphaGain() const {
    return pressure_alpha_gain;
}

qreal Brush::getPressureAspectRatioGain() const {
    return pressure_aspect_ratio_gain;
}

qreal Brush::getSpeedRadiusGain() const {
    return speed_radius_gain;
}

qreal Brush::getSpeedOpacityGain() const {
    return speed_opacity_gain;
}

qreal Brush::getSpeedHardnessGain() const {
    return speed_hardness_gain;
}
