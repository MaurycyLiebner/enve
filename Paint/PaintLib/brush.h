#ifndef BRUSH_H
#define BRUSH_H
#include "GL/gl.h"
#include <QString>
#include <QList>

enum BrushSetting {
    BRUSH_SETTING_RADIUS,
    BRUSH_SETTING_OPACITY,
    BRUSH_SETTING_HARDNESS,

    BRUSH_SETTING_ASPECT_RATIO,
    BRUSH_SETTING_DISTANCE_BETWEEN_DABS,
    BRUSH_SETTING_INITIAL_ROTATION,
    BRUSH_SETTING_ROTATION_INFLUENCE,
    BRUSH_SETTING_ROTATION_DELAY,
    BRUSH_SETTING_ROTATION_BETWEEN_DABS,
    BRUSH_SETTING_INITIAL_ALPHA,
    BRUSH_SETTING_COLOR_PICK_UP,
    BRUSH_SETTING_COLOR_DECAY,
    BRUSH_SETTING_DAB_POSITION_NOISE,
    BRUSH_SETTING_STROKE_POSITION_NOISE,
    BRUSH_SETTING_STROKE_POSITION_NOISE_FREQUENCY,

    BRUSH_SETTING_ROTATION_NOISE,
    BRUSH_SETTING_ROTATION_NOISE_FREQUENCY,

    BRUSH_SETTING_HUE_NOISE,
    BRUSH_SETTING_HUE_NOISE_FREQUENCY,

    BRUSH_SETTING_SATURATION_NOISE,
    BRUSH_SETTING_SATURATION_NOISE_FREQUENCY,

    BRUSH_SETTING_VALUE_NOISE,
    BRUSH_SETTING_VALUE_NOISE_FREQUENCY,

    BRUSH_SETTING_PRESSURE_RADIUS_GAIN,
    BRUSH_SETTING_PRESSURE_OPACITY_GAIN,
    BRUSH_SETTING_PRESSURE_HARDNESS_GAIN,
    BRUSH_SETTING_PRESSURE_ALPHA_GAIN,
    BRUSH_SETTING_PRESSURE_ASCPECT_RATIO_GAIN,

    BRUSH_SETTING_SPEED_RADIUS_GAIN,
    BRUSH_SETTING_SPEED_OPACITY_GAIN,
    BRUSH_SETTING_SPEED_HARDNESS_GAIN,

    BRUSH_SETTINGS_COUNT
};

struct BrushSettingInfo
{
    BrushSetting setting;
    qreal min;
    qreal max;
    qreal def;
    QString name;

    BrushSettingInfo(BrushSetting setting_t,
                     qreal min_t, qreal max_t,
                     qreal def_t, QString name_t) {
        setting = setting_t;
        min = min_t;
        max = max_t;
        def = def_t;
        name = name_t;
    }
};

class Brush {
public:
    Brush();

    static qreal getRed() {
        return red;
    }

    static qreal getGreen() {
        return green;
    }

    static qreal getBlue() {
        return blue;
    }

    static qreal getAlpha() {
        return alpha;
    }

    static void setHSV(qreal h_t, qreal s_t, qreal v_t);
    static void getRGBA(qreal *red_t,
                 qreal *green_t,
                 qreal *blue_t,
                 qreal *alpha_t);
    static void getHSV(qreal *h_t,
                qreal *s_t,
                qreal *v_t);

    qreal *getSettingValuePointer(const BrushSetting &setting_t);
    const qreal *getSettingValuePointerConst(const BrushSetting &setting_t) const;
    qreal getDistBetweenDabsPx() const;

    void setSetting(BrushSetting setting_t, qreal val_t);
    void loadDefaultSettings();
    void writeBrush(QIODevice *write);
    void readBrush(QIODevice *read);
    qreal getSettingVal(BrushSetting setting_t) const;
    static const BrushSettingInfo *getBrushSettingInfo(BrushSetting setting_t);

    qreal getHardness() const;
    qreal getOpacity() const;
    qreal getRadius() const;
    qreal getAspectRatio() const;
    qreal getColorPickUp() const;
    qreal getColorPickUpDecay() const;
    qreal getInitialRotation() const;
    qreal getRotationInfluence() const;
    qreal getRotationDelay() const;
    qreal getDabPositionNoisePx() const;
    qreal getStrokePositionNoisePx() const;
    qreal getStrokePositionNoiseFrequency() const;

    qreal getRotationNoise() const;
    qreal getRotationNoiseFrequency() const;

    qreal getHueNoise() const;
    qreal getHueNoiseFrequency() const;
    qreal getSaturationNoise() const;
    qreal getSaturationNoiseFrequency() const;
    qreal getValueNoise() const;
    qreal getValueNoiseFrequency() const;

    qreal getRotationBetweenDabs() const;

    qreal getPressureRadiusGainPx() const;
    qreal getPressureOpacityGain() const;
    qreal getPressureHardnessGain() const;
    qreal getPressureAlphaGain() const;
    qreal getPressureAspectRatioGain() const;

    qreal getSpeedRadiusGain() const;
    qreal getSpeedOpacityGain() const;
    qreal getSpeedHardnessGain() const;

    void setSettingFromString(QString setting_line_t);
    static BrushSettingInfo brush_settings_info[];
    QString getSettingAsFileLine(BrushSetting setting_t) const;

    void setBrushName(QString brush_name_t);
    void setCollectionName(QString collection_name_t);
    void setBrushFilePath(QString path_t);

    QString getBrushName() const;
    QString getCollectionName() const;
    QString getBrushFilePath() const;

    qreal getDistBetweenDabs() const;
    qreal getPressureRadiusGain() const;

    void setStrokeBrushFilePath(QString path_t);
    QString getStrokeBrushFilePath() const;
    void setChangedSinceLastStroke(bool b_t);
    bool wasChangedSinceLastStroke() const;

    qreal getInitialAlpha() const {
        return initial_alpha;
    }

    void setProjectOnly(const bool &bT) {
        mIsProjectOnly = bT;
    }

    const bool &isProjectOnly() const {
        return mIsProjectOnly;
    }
private:
    bool mIsProjectOnly = true;
    bool changed_since_last_stroke = true;

    QString stroke_brush_file_path;

    QString brush_file_path;

    QString brush_name;
    QString collection_name;

    static qreal h;
    static qreal s;
    static qreal v;

    static qreal red;
    static qreal green;
    static qreal blue;
    static qreal alpha;
    // alpha (weight) of brush color throughout painting
    qreal initial_alpha;
    qreal hardness;
    qreal opacity;
    qreal radius;
    qreal aspect_ratio;
    qreal distance_between_dabs;
    qreal initial_rotation;
    qreal rotation_influence;
    qreal rotation_delay;
    qreal rotation_between_dabs;
    // how much color is picked up from surface
    qreal color_pick_up;
    // how much smudge(picked up) color decays with every stroke
    qreal color_decay;

    qreal dab_position_noise;
    qreal stroke_position_noise;
    qreal stroke_position_noise_frequency;

    qreal rotation_noise;
    qreal rotation_noise_frequency;

    qreal hue_noise;
    qreal hue_noise_frequency;
    qreal saturation_noise;
    qreal saturation_noise_frequency;
    qreal value_noise;
    qreal value_noise_frequency;

    qreal pressure_radius_gain;
    qreal pressure_opacity_gain;
    qreal pressure_hardness_gain;
    qreal pressure_alpha_gain;
    qreal pressure_aspect_ratio_gain;

    qreal speed_radius_gain;
    qreal speed_opacity_gain;
    qreal speed_hardness_gain;
};

#endif // BRUSH_H
