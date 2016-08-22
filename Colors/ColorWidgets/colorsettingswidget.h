#ifndef COLORSETTINGSWIDGET_H
#define COLORSETTINGSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include "h_wheel_sv_triangle.h"
#include "colorvaluerect.h"
#include "colorlabel.h"
#include <QTabWidget>
#include <QLabel>
#include "colorvaluespin.h"
#include <QPushButton>

class ColorSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSettingsWidget(QWidget *parent = 0);


    void setCurrentColor(Color color);
signals:
    void colorChangedHSVSignal(GLfloat, GLfloat, GLfloat, GLfloat);
public slots:
    void setCurrentColor(GLfloat h_t, GLfloat s_t, GLfloat v_t, GLfloat a_t = 1.f);
    void colorChangedHSVSlot(GLfloat h_t, GLfloat s_t, GLfloat v_t);
private slots:
    void alphaChanged(GLfloat a_t);

    void moveAlphaWidgetToTab(int tabId);

    void startColorPicking();
private:
    void connectSignalsAndSlots();

    QTabWidget *mTabWidget = new QTabWidget();
    QVBoxLayout *mWidgetsLayout = new QVBoxLayout();

    QWidget *mWheelWidget = new QWidget();
    QVBoxLayout *mWheelLayout = new QVBoxLayout();
    H_Wheel_SV_Triangle *wheel_triangle_widget = NULL;

    QWidget *mRGBWidget = new QWidget();
    QVBoxLayout *mRGBLayout = new QVBoxLayout();
    QHBoxLayout *rLayout = new QHBoxLayout();
    QLabel *rLabel = new QLabel("R:");
    ColorValueRect *r_rect = NULL;
    ColorValueSpin *rSpin = new ColorValueSpin(0, 255, 0, this);
    QHBoxLayout *gLayout = new QHBoxLayout();
    QLabel *gLabel = new QLabel("G:");
    ColorValueRect *g_rect = NULL;
    ColorValueSpin *gSpin = new ColorValueSpin(0, 255, 0, this);
    QHBoxLayout *bLayout = new QHBoxLayout();
    QLabel *bLabel = new QLabel("B:");
    ColorValueRect *b_rect = NULL;
    ColorValueSpin *bSpin = new ColorValueSpin(0, 255, 0, this);

    QWidget *mHSVWidget = new QWidget();
    QVBoxLayout *mHSVLayout = new QVBoxLayout();
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *hLabel = new QLabel("H:");
    ColorValueRect *h_rect = NULL;
    ColorValueSpin *hSpin = new ColorValueSpin(0, 360, 0, this);
    QHBoxLayout *hsvSLayout = new QHBoxLayout();
    QLabel *hsvSLabel = new QLabel("S:");
    ColorValueRect *hsv_s_rect = NULL;
    ColorValueSpin *hsvSSpin = new ColorValueSpin(0, 100, 0, this);
    QHBoxLayout *vLayout = new QHBoxLayout();
    QLabel *vLabel = new QLabel("V:");
    ColorValueRect *v_rect = NULL;
    ColorValueSpin *vSpin = new ColorValueSpin(0, 100, 0, this);

    QWidget *mHSLWidget = new QWidget();
    QVBoxLayout *mHSLLayout = new QVBoxLayout();
    QHBoxLayout *hslHLayout = new QHBoxLayout();
    QLabel *hslHLabel = new QLabel("H:");
    ColorValueRect *hsl_h_rect = NULL;
    ColorValueSpin *hslHSpin = new ColorValueSpin(0, 360, 0, this);
    QHBoxLayout *hslSLayout = new QHBoxLayout();
    QLabel *hslSLabel = new QLabel("S:");
    ColorValueRect *hsl_s_rect = NULL;
    ColorValueSpin *hslSSpin = new ColorValueSpin(0, 100, 0, this);
    QHBoxLayout *lLayout = new QHBoxLayout();
    QLabel *lLabel = new QLabel("L:");
    ColorValueRect *l_rect = NULL;
    ColorValueSpin *lSpin = new ColorValueSpin(0, 100, 0, this);

    QHBoxLayout *aLayout = new QHBoxLayout();
    QLabel *aLabel = new QLabel("A:");
    ColorValueRect *a_rect = NULL;
    ColorValueSpin *aSpin = new ColorValueSpin(0, 100, 100, this);

    QHBoxLayout *mColorLabelLayout = new QHBoxLayout();
    QPushButton *mPickingButton;

    ColorLabel *color_label = NULL;
    void connectColorWidgetSignalToSlot(ColorWidget *slot_obj, const char *slot,
                                        ColorWidget *signal_src, const char *signal);
    void connectColorWidgetSignalToSlots(ColorWidget *signal_src,
                                         const char *signal, const char *slot );
};

#endif // COLORSETTINGSWIDGET_H
