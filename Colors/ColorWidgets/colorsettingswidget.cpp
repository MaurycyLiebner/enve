#include "colorsettingswidget.h"
#include "mainwindow.h"
#include <QResizeEvent>
#include <QMenu>

void moveAndResizeValueRect(int rect_x_t, int *rect_y_t,
                            int rect_width, int rect_height,
                            ColorValueRect *rect_t)
{
    if(rect_t->isHidden() )
    {
        return;
    }
    rect_t->move(rect_x_t, *rect_y_t);
    *rect_y_t += rect_height;
    rect_t->resize(rect_width, rect_height);
}

void ColorSettingsWidget::setCurrentColor(GLfloat h_t, GLfloat s_t, GLfloat v_t, GLfloat a_t)
{
    wheel_triangle_widget->setColorHSV_f(h_t, s_t, v_t);
    r_rect->setColorHSV_f(h_t, s_t, v_t);
    g_rect->setColorHSV_f(h_t, s_t, v_t);
    b_rect->setColorHSV_f(h_t, s_t, v_t);

    h_rect->setColorHSV_f(h_t, s_t, v_t);
    hsv_s_rect->setColorHSV_f(h_t, s_t, v_t);
    v_rect->setColorHSV_f(h_t, s_t, v_t);

    hsl_s_rect->setColorHSV_f(h_t, s_t, v_t);
    l_rect->setColorHSV_f(h_t, s_t, v_t);

    color_label->setColorHSV_f(h_t, s_t, v_t);
    color_label->setAlpha(a_t);

    a_rect->setColorHSL_f(h_t, s_t, v_t);
    a_rect->setVal(a_t);
}

void ColorSettingsWidget::setCurrentColor(Color color)
{
    setCurrentColor(color.gl_h, color.gl_s, color.gl_v, color.gl_a);
}

void ColorSettingsWidget::alphaChanged(GLfloat a_t)
{
    emit colorChangedHSVSignal(h_rect->getVal(),
                               hsv_s_rect->getVal(),
                               v_rect->getVal(),
                               a_t);
}

void ColorSettingsWidget::colorChangedHSVSlot(GLfloat h_t, GLfloat s_t, GLfloat v_t)
{
    emit colorChangedHSVSignal(h_t, s_t, v_t, a_rect->getVal());
}

void ColorSettingsWidget::moveAlphaWidgetToTab(int tabId)
{
    ((QVBoxLayout*)aLayout->parent())->removeItem(aLayout);
    if(tabId == 0) {
        mRGBLayout->addLayout(aLayout);
    } else if(tabId == 1) {
        mHSVLayout->addLayout(aLayout);
    } else if(tabId == 2) {
        mHSLLayout->addLayout(aLayout);
    } else if(tabId == 3) {
        mWheelLayout->addLayout(aLayout);
    }
    for(int i=0;i < mTabWidget->count();i++)
        if(i!=tabId)
            mTabWidget->widget(i)->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);

    mTabWidget->widget(tabId)->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    mTabWidget->widget(tabId)->resize(mTabWidget->widget(tabId)->minimumSizeHint());
}

ColorSettingsWidget::ColorSettingsWidget(QWidget *parent) : QWidget(parent)
{
    mWidgetsLayout->setAlignment(Qt::AlignTop);
    setLayout(mWidgetsLayout);

    color_label = new ColorLabel(this);

    mWheelWidget->setLayout(mWheelLayout);
    mWheelLayout->setAlignment(Qt::AlignTop);
    wheel_triangle_widget = new H_Wheel_SV_Triangle(this);
    mWheelLayout->addWidget(wheel_triangle_widget, Qt::AlignHCenter);
    mWheelLayout->setAlignment(wheel_triangle_widget, Qt::AlignHCenter);

    int LABEL_WIDTH = 20;

    r_rect = new ColorValueRect(CVR_RED, this);
    rLabel->setFixedWidth(LABEL_WIDTH);
    rLayout->addWidget(rLabel);
    rLayout->addWidget(r_rect);
    rLayout->addWidget(rSpin->getSpinBox());
    g_rect = new ColorValueRect(CVR_GREEN, this);
    gLabel->setFixedWidth(LABEL_WIDTH);
    gLayout->addWidget(gLabel);
    gLayout->addWidget(g_rect);
    gLayout->addWidget(gSpin->getSpinBox());
    b_rect = new ColorValueRect(CVR_BLUE, this);
    bLabel->setFixedWidth(LABEL_WIDTH);
    bLayout->addWidget(bLabel);
    bLayout->addWidget(b_rect);
    bLayout->addWidget(bSpin->getSpinBox());
    mRGBLayout->setAlignment(Qt::AlignTop);
    mRGBLayout->addLayout(rLayout);
    mRGBLayout->addLayout(gLayout);
    mRGBLayout->addLayout(bLayout);
    mRGBWidget->setLayout(mRGBLayout);

    h_rect = new ColorValueRect(CVR_HUE, this);
    hLabel->setFixedWidth(LABEL_WIDTH);
    hLayout->addWidget(hLabel);
    hLayout->addWidget(h_rect);
    hLayout->addWidget(hSpin->getSpinBox());
    hsv_s_rect = new ColorValueRect(CVR_HSVSATURATION, this);
    hsvSLabel->setFixedWidth(LABEL_WIDTH);
    hsvSLayout->addWidget(hsvSLabel);
    hsvSLayout->addWidget(hsv_s_rect);
    hsvSLayout->addWidget(hsvSSpin->getSpinBox());
    v_rect = new ColorValueRect(CVR_VALUE, this);
    vLabel->setFixedWidth(LABEL_WIDTH);
    vLayout->addWidget(vLabel);
    vLayout->addWidget(v_rect);
    vLayout->addWidget(vSpin->getSpinBox());
    mHSVLayout->setAlignment(Qt::AlignTop);
    mHSVLayout->addLayout(hLayout);
    mHSVLayout->addLayout(hsvSLayout);
    mHSVLayout->addLayout(vLayout);
    mHSVWidget->setLayout(mHSVLayout);

    hsl_h_rect = new ColorValueRect(CVR_HUE, this);
    hslHLabel->setFixedWidth(LABEL_WIDTH);
    hslHLayout->addWidget(hslHLabel);
    hslHLayout->addWidget(hsl_h_rect);
    hslHLayout->addWidget(hslHSpin->getSpinBox());
    hsl_s_rect = new ColorValueRect(CVR_HSLSATURATION, this);
    hslSLabel->setFixedWidth(LABEL_WIDTH);
    hslSLayout->addWidget(hslSLabel);
    hslSLayout->addWidget(hsl_s_rect);
    hslSLayout->addWidget(hslSSpin->getSpinBox());
    l_rect = new ColorValueRect(CVR_LIGHTNESS, this);
    lLabel->setFixedWidth(LABEL_WIDTH);
    lLayout->addWidget(lLabel);
    lLayout->addWidget(l_rect);
    lLayout->addWidget(lSpin->getSpinBox());
    mHSLLayout->setAlignment(Qt::AlignTop);
    mHSLLayout->addLayout(hslHLayout);
    mHSLLayout->addLayout(hslSLayout);
    mHSLLayout->addLayout(lLayout);
    mHSLWidget->setLayout(mHSLLayout);

    a_rect = new ColorValueRect(CVR_ALPHA, this);
    aLabel->setFixedWidth(LABEL_WIDTH);
    aLayout->addWidget(aLabel);
    aLayout->addWidget(a_rect);
    aLayout->addWidget(aSpin->getSpinBox());

    mWidgetsLayout->addWidget(color_label);
    mTabWidget->addTab(mRGBWidget, "RGB");
    mTabWidget->addTab(mHSVWidget, "HSV");
    mTabWidget->addTab(mHSLWidget, "HSL");
    mTabWidget->addTab(mWheelWidget, "Wheel");
    mWidgetsLayout->addWidget(mTabWidget);
    mRGBLayout->addLayout(aLayout);

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(moveAlphaWidgetToTab(int)));

    connectSignalsAndSlots();

    setMinimumSize(250, 200);
    mTabWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    setCurrentColor(0.f, 0.f, 0.f);

    moveAlphaWidgetToTab(0);
}

void ColorSettingsWidget::connectColorWidgetSignalToSlot(ColorWidget *slot_obj, const char *slot,
                                                         ColorWidget *signal_src, const char *signal)
{
    if(slot_obj == signal_src)
    {
        return;
    }
    connect(signal_src, signal, slot_obj, slot );
}

void ColorSettingsWidget::connectColorWidgetSignalToSlots(ColorWidget *signal_src,
                                                          const char *signal, const char *slot)
{
    connectColorWidgetSignalToSlot(r_rect, slot, signal_src, signal);
    connectColorWidgetSignalToSlot(g_rect, slot, signal_src, signal);
    connectColorWidgetSignalToSlot(b_rect, slot, signal_src, signal);

    connectColorWidgetSignalToSlot(h_rect, slot, signal_src, signal);
    connectColorWidgetSignalToSlot(hsv_s_rect, slot, signal_src, signal);
    connectColorWidgetSignalToSlot(v_rect, slot, signal_src, signal);

    connectColorWidgetSignalToSlot(hsl_h_rect, slot, signal_src, signal);
    connectColorWidgetSignalToSlot(hsl_s_rect, slot, signal_src, signal);
    connectColorWidgetSignalToSlot(l_rect, slot, signal_src, signal);

    connectColorWidgetSignalToSlot(wheel_triangle_widget, slot, signal_src, signal);

    connectColorWidgetSignalToSlot(color_label, slot, signal_src, signal);

    connectColorWidgetSignalToSlot(a_rect, slot, signal_src, signal);

    connect(signal_src, signal, this, SLOT(colorChangedHSVSlot(GLfloat, GLfloat, GLfloat)) );
}

void addColorWidgetActionToMenu(QMenu *menu_t, QString label_t, ColorWidget *widget_t)
{
    QAction *action_t = menu_t->addAction(label_t);
    action_t->setCheckable(true);
    action_t->setChecked(widget_t->isVisible() );
}

void connectRectToSpin(ColorValueRect *rect, ColorValueSpin *spin) {
    QObject::connect(rect, SIGNAL(valChanged(GLfloat)),
            spin, SLOT(setVal(GLfloat)) );
    QObject::connect(rect, SIGNAL(valUpdated(GLfloat)),
            spin, SLOT(setVal(GLfloat)) );
    QObject::connect(spin, SIGNAL(valSet(GLfloat)),
            rect, SLOT(setValAndEmitSignal(GLfloat)) );
}

void ColorSettingsWidget::connectSignalsAndSlots()
{
    // connect wheel_triangle_widget signal to slots
    connectColorWidgetSignalToSlots(wheel_triangle_widget, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //

    // connect r_rect singla to slots
    connectColorWidgetSignalToSlots(r_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(r_rect, rSpin);

    // connect g_rect singla to slots
    connectColorWidgetSignalToSlots(g_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(g_rect, gSpin);


    // connect b_rect singla to slots
    connectColorWidgetSignalToSlots(b_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(b_rect, bSpin);

    // connect h_rect singla to slots
    connectColorWidgetSignalToSlots(h_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(h_rect, hSpin);

    // connect hsv_s_rect singla to slots
    connectColorWidgetSignalToSlots(hsv_s_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(hsv_s_rect, hsvSSpin);

    // connect v_rect singla to slots
    connectColorWidgetSignalToSlots(v_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(v_rect, vSpin);

    // connect h_rect singla to slots
    connectColorWidgetSignalToSlots(hsl_h_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(hsl_h_rect, hslHSpin);

    // connect hsl_s_rect singla to slots
    connectColorWidgetSignalToSlots(hsl_s_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(hsl_s_rect, hslSSpin);

    // connect l_rect singla to slots
    connectColorWidgetSignalToSlots(l_rect, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //
    connectRectToSpin(l_rect, lSpin);

    // connect color_label singla to slots
    connectColorWidgetSignalToSlots(color_label, SIGNAL(colorChangedHSV(GLfloat,GLfloat,GLfloat)),
                                    SLOT(setColorHSV_f(GLfloat, GLfloat, GLfloat) ) );
    //

    connectRectToSpin(a_rect, aSpin);

    connect(a_rect, SIGNAL(valChanged(GLfloat)), color_label, SLOT(setAlpha(GLfloat)));

    connect(a_rect, SIGNAL(valChanged(GLfloat)),
            this, SLOT(alphaChanged(GLfloat)));
}
