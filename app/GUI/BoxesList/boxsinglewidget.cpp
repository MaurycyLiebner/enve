#include "boxsinglewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "GUI/ColorWidgets/colorsettingswidget.h"

#include "Boxes/layerbox.h"
#include "GUI/qrealanimatorvalueslider.h"
#include "boxscrollwidgetvisiblepart.h"
#include "GUI/keysview.h"
#include "pointhelpers.h"
#include "GUI/BoxesList/boolpropertywidget.h"
#include "boxtargetwidget.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/comboboxproperty.h"
#include "Animators/qstringanimator.h"
#include "Animators/randomqrealgenerator.h"
#include "Properties/boolproperty.h"
#include "Properties/intproperty.h"
#include "Animators/qpointfanimator.h"
#include "PropertyUpdaters/propertyupdater.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/pathbox.h"

#include "typemenu.h"

QPixmap* BoxSingleWidget::VISIBLE_PIXMAP;
QPixmap* BoxSingleWidget::INVISIBLE_PIXMAP;
QPixmap* BoxSingleWidget::HIDE_CHILDREN;
QPixmap* BoxSingleWidget::SHOW_CHILDREN;
QPixmap* BoxSingleWidget::LOCKED_PIXMAP;
QPixmap* BoxSingleWidget::UNLOCKED_PIXMAP;
QPixmap* BoxSingleWidget::ANIMATOR_CHILDREN_VISIBLE;
QPixmap* BoxSingleWidget::ANIMATOR_CHILDREN_HIDDEN;
QPixmap* BoxSingleWidget::ANIMATOR_RECORDING;
QPixmap* BoxSingleWidget::ANIMATOR_NOT_RECORDING;
QPixmap* BoxSingleWidget::ANIMATOR_DESCENDANT_RECORDING;
bool BoxSingleWidget::sStaticPixmapsLoaded = false;

#include "global.h"
#include "GUI/mainwindow.h"
#include "clipboardcontainer.h"
#include "durationrectangle.h"
#include "boxeslistactionbutton.h"
#include "coloranimatorbutton.h"
#include "canvas.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
#include "Animators/fakecomplexanimator.h"

#include <QApplication>
#include <QDrag>
#include <QMenu>
#include <QInputDialog>

BoxSingleWidget::BoxSingleWidget(ScrollWidgetVisiblePart *parent) :
    SingleWidget(parent) {
    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    mMainLayout->setMargin(0);
    mMainLayout->setAlignment(Qt::AlignLeft);

    mRecordButton = new PixmapActionButton(this);
    mRecordButton->setPixmapChooser([this]() {
        if(!mTarget) return static_cast<QPixmap*>(nullptr);
        const auto target = mTarget->getTarget();
        if(target->SWT_isBoundingBox()) {
            return static_cast<QPixmap*>(nullptr);
        } else if(target->SWT_isComplexAnimator()) {
            const auto caTarget = GetAsPtr(target, ComplexAnimator);
            if(caTarget->anim_isRecording()) {
                return BoxSingleWidget::ANIMATOR_RECORDING;
            } else {
                if(caTarget->anim_isDescendantRecording()) {
                    return BoxSingleWidget::ANIMATOR_DESCENDANT_RECORDING;
                } else return BoxSingleWidget::ANIMATOR_NOT_RECORDING;
            }
        } else if(target->SWT_isAnimator()) {
            if(GetAsPtr(target, Animator)->anim_isRecording()) {
                return BoxSingleWidget::ANIMATOR_RECORDING;
            } else return BoxSingleWidget::ANIMATOR_NOT_RECORDING;
        } else return static_cast<QPixmap*>(nullptr);
    });

    mMainLayout->addWidget(mRecordButton);
    connect(mRecordButton, &BoxesListActionButton::pressed,
            this, &BoxSingleWidget::switchRecordingAction);

    mContentButton = new PixmapActionButton(this);
    mContentButton->setPixmapChooser([this]() {
        if(!mTarget) return static_cast<QPixmap*>(nullptr);
        const auto target = mTarget->getTarget();
        if(target->SWT_isBoundingBox()) {
            if(mTarget->contentVisible()) {
                return BoxSingleWidget::HIDE_CHILDREN;
            } else return BoxSingleWidget::SHOW_CHILDREN;
        } else {
            if(mTarget->contentVisible()) {
                return BoxSingleWidget::ANIMATOR_CHILDREN_VISIBLE;
            } else {
                return BoxSingleWidget::ANIMATOR_CHILDREN_HIDDEN;
            }
        }
    });

    mMainLayout->addWidget(mContentButton);
    connect(mContentButton, &BoxesListActionButton::pressed,
            this, &BoxSingleWidget::switchContentVisibleAction);

    mVisibleButton = new PixmapActionButton(this);
    mVisibleButton->setPixmapChooser([this]() {
        if(!mTarget) return static_cast<QPixmap*>(nullptr);
        const auto target = mTarget->getTarget();
        if(target->SWT_isBoundingBox()) {
            if(GetAsPtr(target, BoundingBox)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else if(target->SWT_isPixmapEffect()) {
            if(GetAsPtr(target, PixmapEffect)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else if(target->SWT_isPathEffect()) {
            if(GetAsPtr(target, PathEffect)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else return static_cast<QPixmap*>(nullptr);
    });
    mMainLayout->addWidget(mVisibleButton);
    connect(mVisibleButton, &BoxesListActionButton::pressed,
            this, &BoxSingleWidget::switchBoxVisibleAction);

    mLockedButton = new PixmapActionButton(this);
    mLockedButton->setPixmapChooser([this]() {
        if(!mTarget) return static_cast<QPixmap*>(nullptr);
        const auto target = mTarget->getTarget();
        if(target->SWT_isBoundingBox()) {
            if(GetAsPtr(target, BoundingBox)->isLocked()) {
                return BoxSingleWidget::LOCKED_PIXMAP;
            } else return BoxSingleWidget::UNLOCKED_PIXMAP;
        } else return static_cast<QPixmap*>(nullptr);
    });

    mMainLayout->addWidget(mLockedButton);
    connect(mLockedButton, &BoxesListActionButton::pressed,
            this, &BoxSingleWidget::switchBoxLockedAction);

    mFillWidget = new QWidget(this);
    mMainLayout->addWidget(mFillWidget);
    mFillWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0)");

    mValueSlider = new QrealAnimatorValueSlider(nullptr, this);
    mMainLayout->addWidget(mValueSlider, Qt::AlignRight);

    mSecondValueSlider = new QrealAnimatorValueSlider(nullptr, this);
    mMainLayout->addWidget(mSecondValueSlider, Qt::AlignRight);

    mColorButton = new ColorAnimatorButton(nullptr, this);
    mMainLayout->addWidget(mColorButton, Qt::AlignRight);
    mColorButton->setFixedHeight(mColorButton->height() - 6);
    mColorButton->setContentsMargins(0, 3, 0, 3);

    mPropertyComboBox = new QComboBox(this);
    mMainLayout->addWidget(mPropertyComboBox);

    mCompositionModeCombo = new QComboBox(this);
    mMainLayout->addWidget(mCompositionModeCombo);
//    mCompositionModeCombo->addItems(QStringList() <<
//                                    "Source Over" <<
//                                    "Destination Over" <<
//                                    "Clear" <<
//                                    "Source" <<
//                                    "Destination" <<
//                                    "Source in" <<
//                                    "Destination in" <<
//                                    "Source Out" <<
//                                    "Destination Out" <<
//                                    "Source Atop" <<
//                                    "Destination Atop" <<
//                                    "Xor" <<
//                                    "Plus" <<
//                                    "Multiply" <<
//                                    "Screen" <<
//                                    "Overlay" <<
//                                    "Darken" <<
//                                    "Lighten" <<
//                                    "Color Burn" <<
//                                    "Hard Light" <<
//                                    "Soft Light" <<
//                                    "Difference" <<
//                                    "Exclusion" <<
//                                    "Source or Destination" <<
//                                    "Source and Destination" <<
//                                    "Source Xor Destination" <<
//                                    "Not Source And Not Destination" <<
//                                    "Not Source or Not Destination" <<
//                                    "Not Source Xor Destination" <<
//                                    "Not Source" <<
//                                    "Not Source And Destination" <<
//                                    "Source And Not Destination" <<
//                                    "Not Source or Destination");
    mCompositionModeCombo->addItems(QStringList() <<
                                    "SrcOver" <<
                                    "DstOver" <<
                                    "SrcIn" <<
                                    "DstIn" <<
                                    "SrcOut" <<
                                    "DstOut" <<
                                    "SrcATop" <<
                                    "DstATop" <<
                                    "Xor" <<
                                    "Plus" <<
                                    "Modulate" <<
                                    "Screen" <<
                                    "Overlay" <<
                                    "Darken" <<
                                    "Lighten" <<
                                    "ColorDodge" <<
                                    "ColorBurn" <<
                                    "HardLight" <<
                                    "SoftLight" <<
                                    "Difference" <<
                                    "Exclusion" <<
                                    "Multiply" <<
                                    "Hue" <<
                                    "Saturation" <<
                                    "Color" <<
                                    "Luminosity");
    mCompositionModeCombo->insertSeparator(10);
    mCompositionModeCombo->insertSeparator(22);
    connect(mCompositionModeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setCompositionMode);
    mCompositionModeCombo->setSizePolicy(QSizePolicy::Maximum,
                    mCompositionModeCombo->sizePolicy().horizontalPolicy());
    mPropertyComboBox->setSizePolicy(QSizePolicy::Maximum,
                                     mPropertyComboBox->sizePolicy().horizontalPolicy());
    mBoxTargetWidget = new BoxTargetWidget(this);
    mMainLayout->addWidget(mBoxTargetWidget);

    mCheckBox = new BoolPropertyWidget(this);
    mMainLayout->addWidget(mCheckBox);

    mMainLayout->addSpacing(MIN_WIDGET_HEIGHT/2);

    hide();
}

SkBlendMode idToBlendModeSk(const int &id) {
    switch(id) {
        case 0: return SkBlendMode::kSrcOver;
        case 1: return SkBlendMode::kDstOver;
        case 2: return SkBlendMode::kSrcIn;
        case 3: return SkBlendMode::kDstIn;
        case 4: return SkBlendMode::kSrcOut;
        case 5: return SkBlendMode::kDstOut;
        case 6: return SkBlendMode::kSrcATop;
        case 7: return SkBlendMode::kDstATop;
        case 8: return SkBlendMode::kXor;
        case 9: return SkBlendMode::kPlus;
        case 10: return SkBlendMode::kModulate;
        case 11: return SkBlendMode::kScreen;
        case 12: return SkBlendMode::kOverlay;
        case 13: return SkBlendMode::kDarken;
        case 14: return SkBlendMode::kLighten;
        case 15: return SkBlendMode::kColorDodge;
        case 16: return SkBlendMode::kColorBurn;
        case 17: return SkBlendMode::kHardLight;
        case 18: return SkBlendMode::kSoftLight;
        case 19: return SkBlendMode::kDifference;
        case 20: return SkBlendMode::kExclusion;
        case 21: return SkBlendMode::kMultiply;
        case 22: return SkBlendMode::kHue;
        case 23: return SkBlendMode::kSaturation;
        case 24: return SkBlendMode::kColor;
        case 25: return SkBlendMode::kLuminosity;
        default: return SkBlendMode::kSrcOver;
    }
}

int blendModeToIntSk(const SkBlendMode &mode) {
    switch(mode) {
        case SkBlendMode::kSrcOver: return 0;
        case SkBlendMode::kDstOver: return 1;
        case SkBlendMode::kSrcIn: return 2;
        case SkBlendMode::kDstIn: return 3;
        case SkBlendMode::kSrcOut: return 4;
        case SkBlendMode::kDstOut: return 5;
        case SkBlendMode::kSrcATop: return 6;
        case SkBlendMode::kDstATop: return 7;
        case SkBlendMode::kXor: return 8;
        case SkBlendMode::kPlus: return 9;
        case SkBlendMode::kModulate: return 10;
        case SkBlendMode::kScreen: return 11;
        case SkBlendMode::kOverlay: return 12;
        case SkBlendMode::kDarken: return 13;
        case SkBlendMode::kLighten: return 14;
        case SkBlendMode::kColorDodge: return 15;
        case SkBlendMode::kColorBurn: return 16;
        case SkBlendMode::kHardLight: return 17;
        case SkBlendMode::kSoftLight: return 18;
        case SkBlendMode::kDifference: return 19;
        case SkBlendMode::kExclusion: return 20;
        case SkBlendMode::kMultiply: return 21;
        case SkBlendMode::kHue: return 22;
        case SkBlendMode::kSaturation: return 23;
        case SkBlendMode::kColor: return 24;
        case SkBlendMode::kLuminosity: return 25;
        default: return 0;
    }
}

void BoxSingleWidget::setCompositionMode(const int &id) {
    SingleWidgetTarget *target = mTarget->getTarget();

    if(target->SWT_isBoundingBox()) {
        auto boxTarget = GetAsPtr(target, BoundingBox);
        boxTarget->setBlendModeSk(idToBlendModeSk(id));
    }
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

ColorAnimator *BoxSingleWidget::getColorTarget() const {
    const auto swt = mTarget->getTarget();
    ColorAnimator * color = nullptr;
    if(swt->SWT_isColorAnimator()) {
        color = GetAsPtr(mTarget->getTarget(), ColorAnimator);
    } else if(swt->SWT_isComplexAnimator()) {
        const auto ca = GetAsPtr(mTarget->getTarget(), ComplexAnimator);
        color = GetAsPtr(ca->getPropertyForGUI(), ColorAnimator);
    }
    return color;
}

void BoxSingleWidget::setBlendMode(const SkBlendMode &mode) {
    int id = blendModeToIntSk(mode);
    mCompositionModeCombo->setCurrentIndex(id);
}

void BoxSingleWidget::clearAndHideValueAnimators() {
    mValueSlider->clearTarget();
    mValueSlider->hide();
    mSecondValueSlider->clearTarget();
    mSecondValueSlider->hide();
}

void BoxSingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    SingleWidget::setTargetAbstraction(abs);
    SingleWidgetTarget *target = abs->getTarget();

    mContentButton->setVisible(target->SWT_isComplexAnimator());
    if(target->SWT_isFakeComplexAnimator()) {
        target = GetAsPtr(target, FakeComplexAnimator)->getTarget();
    }
    mRecordButton->setVisible(target->SWT_isAnimator());
    mVisibleButton->setVisible(target->SWT_isBoundingBox() ||
                               target->SWT_isPathEffect() ||
                               target->SWT_isPixmapEffect());
    mLockedButton->setVisible(target->SWT_isBoundingBox());
    mRecordButton->show();

    mCompositionModeCombo->hide();
    mPropertyComboBox->hide();

    mCompositionModeVisible = false;

    mBoxTargetWidget->hide();
    mCheckBox->hide();

    clearColorButton();
    clearAndHideValueAnimators();

    if(target->SWT_isBoundingBox()) {
        mRecordButton->hide();
        const auto boxPtr = GetAsPtr(target, BoundingBox);

        mCompositionModeVisible = true;
        mCompositionModeCombo->setCurrentIndex(
            blendModeToIntSk(boxPtr->getBlendMode()));
        updateCompositionBoxVisible();
    } else if(target->SWT_isBoolProperty()) {
        mCheckBox->show();
        mCheckBox->setTarget(GetAsPtr(target, BoolProperty));
    } else if(target->SWT_isBoolPropertyContainer()) {
        mCheckBox->show();
        mCheckBox->setTarget(GetAsPtr(target, BoolPropertyContainer));
    } else if(target->SWT_isComboBoxProperty()) {
        disconnect(mPropertyComboBox, nullptr, nullptr, nullptr);
        if(mLastComboBoxProperty.data() != nullptr) {
            disconnect(mLastComboBoxProperty.data(), nullptr,
                       mPropertyComboBox, nullptr);
        }
        auto comboBoxProperty = GetAsPtr(target, ComboBoxProperty);
        mLastComboBoxProperty = comboBoxProperty;
        mPropertyComboBox->clear();
        mPropertyComboBox->addItems(comboBoxProperty->getValueNames());
        mPropertyComboBox->setCurrentIndex(
                    comboBoxProperty->getCurrentValue());
        mPropertyComboBox->show();
        connect(mPropertyComboBox,
                qOverload<int>(&QComboBox::activated),
                comboBoxProperty, &ComboBoxProperty::setCurrentValue);
        connect(comboBoxProperty, &ComboBoxProperty::valueChanged,
                mPropertyComboBox, &QComboBox::setCurrentIndex);
        connect(mPropertyComboBox,
                qOverload<int>(&QComboBox::activated),
                MainWindow::getInstance(),
                &MainWindow::queScheduledTasksAndUpdate);
    } else if(target->SWT_isIntProperty() || target->SWT_isQrealAnimator()) {
        if(target->SWT_isQrealAnimator())
            mValueSlider->setTarget(GetAsPtr(target, QrealAnimator));
        else mValueSlider->setTarget(GetAsPtr(target, IntProperty));
        mValueSlider->show();
        mValueSlider->setNeighbouringSliderToTheRight(false);
    } else if(target->SWT_isComplexAnimator()) {
        if(target->SWT_isColorAnimator()) {
            mColorButton->setColorTarget(GetAsPtr(target, ColorAnimator));
            mColorButton->show();
        }
        if(target->SWT_isComplexAnimator() && !abs->contentVisible()) {
            if(target->SWT_isQPointFAnimator()) {
                updateValueSlidersForQPointFAnimator();
            } else {
                const auto ca_target = GetAsPtr(target, ComplexAnimator);
                Property * const guiProp = ca_target->getPropertyForGUI();
                if(guiProp) {
                    if(guiProp->SWT_isQrealAnimator()) {
                        mValueSlider->setTarget(GetAsPtr(guiProp, QrealAnimator));
                        mValueSlider->show();
                        mValueSlider->setNeighbouringSliderToTheRight(false);
                        mSecondValueSlider->hide();
                        mSecondValueSlider->clearTarget();
                    } else if(guiProp->SWT_isColorAnimator()) {
                        mColorButton->setColorTarget(GetAsPtr(guiProp, ColorAnimator));
                        mColorButton->show();
                    }
                }
            }
        }
    } else if(target->SWT_isBoxTargetProperty()) {
        mBoxTargetWidget->show();
        mBoxTargetWidget->setTargetProperty(
                    GetAsPtr(target, BoxTargetProperty));
    }
}

void BoxSingleWidget::loadStaticPixmaps() {
    if(sStaticPixmapsLoaded) return;
    VISIBLE_PIXMAP = new QPixmap(":/icons/visible.png");
    INVISIBLE_PIXMAP = new QPixmap(":/icons/hidden.png");
    HIDE_CHILDREN = new QPixmap(":/icons/list_hide_children.png");
    SHOW_CHILDREN = new QPixmap(":/icons/list_show_children.png");
    LOCKED_PIXMAP = new QPixmap(":/icons/lock_locked.png");
    UNLOCKED_PIXMAP = new QPixmap(":/icons/lock_unlocked.png");
    ANIMATOR_CHILDREN_VISIBLE = new QPixmap(
                ":/icons/animator_children_visible.png");
    ANIMATOR_CHILDREN_HIDDEN = new QPixmap(
                ":/icons/animator_children_hidden.png");
    ANIMATOR_RECORDING = new QPixmap(
                ":/icons/recording.png");
    ANIMATOR_NOT_RECORDING = new QPixmap(
                ":/icons/not_recording.png");
    ANIMATOR_DESCENDANT_RECORDING = new QPixmap(
                ":/icons/desc_recording.png");
    sStaticPixmapsLoaded = true;
}

void BoxSingleWidget::clearStaticPixmaps() {
    if(!sStaticPixmapsLoaded) return;
    delete VISIBLE_PIXMAP;
    delete INVISIBLE_PIXMAP;
    delete HIDE_CHILDREN;
    delete SHOW_CHILDREN;
    delete LOCKED_PIXMAP;
    delete UNLOCKED_PIXMAP;
    delete ANIMATOR_CHILDREN_VISIBLE;
    delete ANIMATOR_CHILDREN_HIDDEN;
    delete ANIMATOR_RECORDING;
    delete ANIMATOR_NOT_RECORDING;
    delete ANIMATOR_DESCENDANT_RECORDING;
}

void BoxSingleWidget::mousePressEvent(QMouseEvent *event) {
    if(isTargetDisabled()) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isFakeComplexAnimator()) {
        target = GetAsPtr(target, FakeComplexAnimator)->getTarget();
    }
    if(event->button() == Qt::RightButton) {
        setSelected(true);
        QMenu menu(this);

        if(target->SWT_isBoundingBox()) {
            BoundingBox *boxTarget = GetAsPtr(target, BoundingBox);
            menu.addAction("Rename", [this]() {
                rename();
            });
            if(!target->SWT_isParticleBox() &&
               !target->SWT_isAnimationBox()) {
                QAction * const durRectAct = menu.addAction("Visibility Range",
                                                            [target]() {
                    auto boxTarget = GetAsPtr(target, BoundingBox);
                    if(boxTarget->hasDurationRectangle()) {
                        boxTarget->setDurationRectangle(nullptr);
                    } else {
                        boxTarget->createDurationRectangle();
                    }
                });
                durRectAct->setCheckable(true);
                durRectAct->setChecked(boxTarget->hasDurationRectangle());
            }
            const auto durRect = boxTarget->getDurationRectangle();
            if(durRect) {
                menu.addAction("Visibility Range Settings...",
                               [this, durRect]() {
                    durRect->openDurationSettingsDialog(this);
                });
            }
            menu.addSeparator();
        }
        if(target->SWT_isProperty()) {
            auto clipboard = MainWindow::getPropertyClipboardContainer();
            menu.addAction("Copy", [target]() {
                if(target->SWT_isBoundingBox()) {
                    auto container = SPtrCreate(BoxesClipboardContainer)();
                    QBuffer targetT(container->getBytesArray());
                    targetT.open(QIODevice::WriteOnly);
                    int nBoxes = 1;
                    targetT.write(rcChar(&nBoxes), sizeof(int));
                    auto boxTarget = GetAsPtr(target, BoundingBox);
                    boxTarget->writeBoundingBox(&targetT);
                    targetT.close();

                    MainWindow::getInstance()->replaceClipboard(container);
                } else {
                    auto container = SPtrCreate(PropertyClipboardContainer)();
                    container->setProperty(GetAsPtr(target, Property));
                    MainWindow::getInstance()->replaceClipboard(container);
                }
                BoundingBox::sClearWriteBoxes();
            });
            if(clipboard) {
                if(target->SWT_isBoundingBox()) {
                    if(target->SWT_isLayerBox() &&
                        !target->SWT_isLinkBox()) {
                        auto boxClip = MainWindow::getBoxesClipboardContainer();
                        if(boxClip) {
                            menu.addAction("Paste Boxes", [target]() {
                                auto clipboard = MainWindow::getBoxesClipboardContainer();
                                clipboard->pasteTo(GetAsPtr(target, LayerBox));
                            });
                        }
                    }
                    if(clipboard->isPathEffect() ||
                        clipboard->isPathEffectAnimators()) {
                        QMenu * const pasteMenu = menu.addMenu("Paste");
                        pasteMenu->addAction("Paste Path Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto targetPathBox = GetAsPtr(target, PathBox);
                            clipboard->paste(targetPathBox->getPathEffectsAnimators());
                        });

                        pasteMenu->addAction("Paste Outline Path Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto targetPathBox = GetAsPtr(target, PathBox);
                            clipboard->paste(targetPathBox->getOutlinePathEffectsAnimators());
                        });

                        pasteMenu->addAction("Paste Fill Path Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto targetPathBox = GetAsPtr(target, PathBox);
                            clipboard->paste(targetPathBox->getFillPathEffectsAnimators());
                        });

                        QMenu * const clearAndPasteMenu = menu.addMenu("Clear and Paste");

                        clearAndPasteMenu->addAction("Clear and Paste Path Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto targetPathBox = GetAsPtr(target, PathBox);
                            clipboard->clearAndPaste(targetPathBox->getPathEffectsAnimators());
                        });

                        clearAndPasteMenu->addAction("Clear and Paste Outline Path Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto targetPathBox = GetAsPtr(target, PathBox);
                            clipboard->clearAndPaste(targetPathBox->getOutlinePathEffectsAnimators());
                        });

                        clearAndPasteMenu->addAction("Clear and Paste Fill Path Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto targetPathBox = GetAsPtr(target, PathBox);
                            clipboard->clearAndPaste(targetPathBox->getFillPathEffectsAnimators());
                        });
                    } else if(clipboard->isPixmapEffect() ||
                              clipboard->isPixmapEffectAnimators()) {
                        menu.addAction("Paste Raster Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto boxTarget = GetAsPtr(target, BoundingBox);
                            clipboard->paste(boxTarget->getEffectsAnimators());
                        });

                        menu.addAction("Clear and Paste Raster Effect", [target]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            auto boxTarget = GetAsPtr(target, BoundingBox);
                            clipboard->clearAndPaste(boxTarget->getEffectsAnimators());
                        });
                    }
                } else {
                    const auto prop = GetAsPtr(target, Property);
                    if(clipboard->propertyCompatible(prop)) {
                        menu.addAction("Paste", [prop]() {
                            auto clipboard = MainWindow::getPropertyClipboardContainer();
                            clipboard->paste(prop);
                        });
                        if(prop->SWT_isAnimator()) {
                            menu.addAction("Clear and Paste", [prop]() {
                                auto clipboard = MainWindow::getPropertyClipboardContainer();
                                clipboard->clearAndPaste(prop);
                            });
                        }
                    } else {
                        menu.addAction("Paste")->setDisabled(true);
                        if(prop->SWT_isAnimator()) {
                            menu.addAction("Clear and Paste")->setDisabled(true);
                        }
                    }
                }
            }
            if(target->SWT_isBoundingBox()) {
                menu.addSeparator();
                const auto boxTarget = GetAsPtr(target, BoundingBox);
                QMenu * const canvasMenu = menu.addMenu("Canvas");
                const auto canvas = boxTarget->getParentCanvas();
                BoxTypeMenu bMenu(canvasMenu, canvas, this);
                canvas->addSelectedBoxesActions(canvasMenu);
            } else if(target->SWT_isAnimator()) {
                menu.addSeparator();
                const auto animTarget = GetAsPtr(target, Animator);
                if(animTarget->anim_getKeyOnCurrentFrame()) {
                    menu.addAction("Add Key")->setDisabled(true);
                    menu.addAction("Delete Key", [animTarget]() {
                        animTarget->anim_deleteCurrentKey();
                    });
                } else {
                    menu.addAction("Add Key", [animTarget]() {
                        animTarget->anim_saveCurrentValueAsKey();
                    });
                    menu.addAction("Delete Key")->setDisabled(true);
                }
                menu.addSeparator();
                if(target->SWT_isPixmapEffect() || target->SWT_isPathEffect()) {
                    menu.addSeparator();
                    menu.addAction("Delete Effect", [target]() {
                        if(target->SWT_isPixmapEffect()) {
                            auto effectTarget = GetAsSPtr(target, PixmapEffect);
                            effectTarget->getParentEffectAnimators()->
                                    getParentBox()->removeEffect(effectTarget);
                        } else {
                            auto effectTarget = GetAsSPtr(target, PathEffect);
                            const auto parentAnimators =
                                    effectTarget->getParent<PathEffectAnimators>();
                            if(parentAnimators->isOutline()) {
                                parentAnimators->getParentBox()->
                                        removeOutlinePathEffect(effectTarget);
                            } else if(parentAnimators->isFill()) {
                                parentAnimators->getParentBox()->
                                        removeFillPathEffect(effectTarget);
                            } else {
                                parentAnimators->getParentBox()->
                                        removePathEffect(effectTarget);
                            }

                        }
                    });
                } else if(target->SWT_isQrealAnimator()) {
                    const auto qrealTarget = GetAsPtr(target, QrealAnimator);
                    if(qrealTarget->hasNoise()) {
                        menu.addSeparator();
                        menu.addAction("Remove Noise", [qrealTarget]() {
                            qrealTarget->setGenerator(nullptr);
                        });
                    } else {
                        menu.addSeparator();
                        menu.addAction("Add Noise", [qrealTarget]() {
                            const auto randGen = SPtrCreate(RandomQrealGenerator)();
                            const auto updater = GetAsSPtr(qrealTarget->prp_getUpdater(),
                                                           PropertyUpdater);
                            randGen->prp_setOwnUpdater(updater);
                            qrealTarget->setGenerator(randGen);
                        });
                    }
                }
            }
        }
        menu.exec(event->globalPos());
        setSelected(false);
    } else {
        mDragStartPos = event->pos();
//        if(type == SWT_BoundingBox ||
//           type == SWT_BoxesGroup) {
//            BoundingBox *bb_target = (BoundingBox*)target;
//            bb_target->selectionChangeTriggered(event->modifiers() &
//                                                Qt::ShiftModifier);
//        }
    }
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

bool BoxSingleWidget::isTargetDisabled() {
    if(!mTarget) return true;
    return mTarget->getTarget()->SWT_isDisabled();
}

void BoxSingleWidget::mouseMoveEvent(QMouseEvent *event) {
    if(!(event->buttons() & Qt::LeftButton)) return;
    if(isTargetDisabled()) return;
    if((event->pos() - mDragStartPos).manhattanLength()
         < QApplication::startDragDistance()) {
        return;
    }
    const auto drag = new QDrag(this);
    connect(drag, &QDrag::destroyed,
            this, &BoxSingleWidget::clearSelected);

    const auto mimeData = mTarget->getTarget()->SWT_createMimeData();
    if(!mimeData) return;
    setSelected(true);
    drag->setMimeData(mimeData);

    drag->installEventFilter(MainWindow::getInstance());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void BoxSingleWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(isTargetDisabled()) return;
    setSelected(false);
    if(pointToLen(event->pos() - mDragStartPos) > MIN_WIDGET_HEIGHT/2) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isBoundingBox() && !target->SWT_isCanvas()) {
        auto boxTarget = GetAsPtr(target, BoundingBox);
        boxTarget->selectionChangeTriggered(event->modifiers() &
                                            Qt::ShiftModifier);
        MainWindow::getInstance()->queScheduledTasksAndUpdate();
    } else if(target->SWT_isGraphAnimator()) {
        auto animTarget = GetAsPtr(target, GraphAnimator);
        auto bsvt = static_cast<BoxScrollWidgetVisiblePart*>(mParent);
        KeysView *keysView = bsvt->getKeysView();
        if(keysView) {
            if(animTarget->graph_isCurrentAnimator(mParent)) {
                keysView->graphRemoveViewedAnimator(animTarget);
            } else {
                keysView->graphAddViewedAnimator(animTarget);
            }
            MainWindow::getInstance()->queScheduledTasksAndUpdate();
        }
    }
}

void BoxSingleWidget::mouseDoubleClickEvent(QMouseEvent *e) {
    if(isTargetDisabled()) return;
    if(e->modifiers() & Qt::ShiftModifier) {
        //mousePressEvent(e);
    } else {
        rename();
        MainWindow::getInstance()->queScheduledTasksAndUpdate();
    }
}

void BoxSingleWidget::rename() {
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isBoundingBox()) {
        const auto boxTarget = GetAsPtr(target, BoundingBox);
        bool ok;
        const QString text = QInputDialog::getText(this, tr("New name dialog"),
                                             tr("Name:"), QLineEdit::Normal,
                                             boxTarget->getName(), &ok);
        if(ok) {
            boxTarget->setName(text);
            boxTarget->SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty();
        }
    }
}

void BoxSingleWidget::drawKeys(QPainter * const p,
                               const qreal &pixelsPerFrame,
                               const FrameRange &viewedFrames) {
    if(isHidden()) return;
    const auto target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        const auto anim_target = static_cast<Animator*>(target);
        anim_target->drawTimelineControls(p, pixelsPerFrame, viewedFrames,
                                          MIN_WIDGET_HEIGHT);
    }
}

Key* BoxSingleWidget::getKeyAtPos(const int &pressX,
                                  const qreal &pixelsPerFrame,
                                  const int &minViewedFrame) {
    if(isHidden()) return nullptr;
    const auto target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        const auto anim_target = static_cast<Animator*>(target);
        return anim_target->anim_getKeyAtPos(pressX,
                                            minViewedFrame,
                                            pixelsPerFrame,
                                            KEY_RECT_SIZE);
    }
    return nullptr;
}

DurationRectangleMovable* BoxSingleWidget::getRectangleMovableAtPos(
                            const int &pressX,
                            const qreal &pixelsPerFrame,
                            const int &minViewedFrame) {
    if(isHidden()) return nullptr;
    const auto target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        const auto anim_target = static_cast<Animator*>(target);
        return anim_target->anim_getTimelineMovable(
                                    pressX,
                                    minViewedFrame,
                                    pixelsPerFrame);
    }
    return nullptr;
}

void BoxSingleWidget::getKeysInRect(const QRectF &selectionRect,
                                    const qreal &pixelsPerFrame,
                                    QList<Key*>& listKeys) {
    if(isHidden()) return;
    const auto target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        const auto anim_target = static_cast<Animator*>(target);

        anim_target->anim_getKeysInRect(selectionRect, pixelsPerFrame,
                                       listKeys, KEY_RECT_SIZE);
    }
}

int BoxSingleWidget::getOptimalNameRightX() {
    if(!mTarget) return 0;
    auto target = mTarget->getTarget();
    bool fakeComplexAnimator = target->SWT_isFakeComplexAnimator();
    if(fakeComplexAnimator) {
        target = static_cast<FakeComplexAnimator*>(target)->getTarget();
    }
    QFontMetrics fm = QFontMetrics(QFont());
    QString name;
    if(target->SWT_isProperty()) {
        name = GetAsPtr(target, Property)->prp_getName();
    }
    int nameX = mFillWidget->x();
    //return nameX;
    if(target->SWT_isBoundingBox()) {
        nameX += MIN_WIDGET_HEIGHT/4;
    } else if(target->SWT_isQrealAnimator()) {
        if(!fakeComplexAnimator) {
            nameX += MIN_WIDGET_HEIGHT;
        }
    } else if(target->SWT_isBoxTargetProperty()) {
        nameX += 2*MIN_WIDGET_HEIGHT;
    } else {//if(target->SWT_isBoolProperty()) {
        nameX += 2*MIN_WIDGET_HEIGHT;
    }
    return nameX + fm.width(name);
}

void BoxSingleWidget::paintEvent(QPaintEvent *) {
    if(!mTarget) return;
    QPainter p(this);
    auto target = mTarget->getTarget();
    if(target->SWT_isDisabled()) {
        p.setOpacity(.5);
    }
    bool fakeComplexAnimator = target->SWT_isFakeComplexAnimator();
    if(fakeComplexAnimator) {
        target = GetAsPtr(target, FakeComplexAnimator)->getTarget();
    }

    int nameX = mFillWidget->x();
    QString name;
    if(target->SWT_isBoundingBox()) {
        const auto bb_target = static_cast<BoundingBox*>(target);

        nameX += MIN_WIDGET_HEIGHT/4;
        name = bb_target->getName();

        p.fillRect(rect(), QColor(0, 0, 0, 50));

        if(bb_target->isSelected()) {
            p.fillRect(mFillWidget->geometry(), QColor(180, 180, 180));
            p.setPen(Qt::black);
        } else {
            p.setPen(Qt::white);
        }
    } else if(!target->SWT_isComplexAnimator()) {
        const auto propTarget = static_cast<Property*>(target);
        if(target->SWT_isGraphAnimator()) {
            auto graphAnim = static_cast<GraphAnimator*>(target);
            if(graphAnim->graph_isCurrentAnimator(mParent)) {
                p.fillRect(nameX + MIN_WIDGET_HEIGHT/4, MIN_WIDGET_HEIGHT/4,
                           MIN_WIDGET_HEIGHT/2, MIN_WIDGET_HEIGHT/2,
                           graphAnim->graph_getAnimatorColor(mParent));
            }
        }
        name = propTarget->prp_getName();
        if(!fakeComplexAnimator) nameX += MIN_WIDGET_HEIGHT;

        p.setPen(Qt::white);
    } else { //if(target->SWT_isComplexAnimator()) {
        ComplexAnimator *caTarget = GetAsPtr(target, ComplexAnimator);
        name = caTarget->prp_getName();

        p.setPen(Qt::white);
    }

    p.drawText(QRect(nameX, 0, width() - nameX -
                     MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT),
               name, QTextOption(Qt::AlignVCenter));
    if(mSelected) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::lightGray));
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
    p.end();
}

void BoxSingleWidget::switchContentVisibleAction() {
    mTarget->switchContentVisible();
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
    //mParent->callUpdaters();
}

void BoxSingleWidget::switchRecordingAction() {
    const auto target = mTarget->getTarget();
    if(!target) return;
    if(!target->SWT_isAnimator()) return;
    auto aTarget = GetAsPtr(target, Animator);
    if(aTarget->SWT_isFakeComplexAnimator()) {
        auto fcaTarget = GetAsPtr(aTarget, FakeComplexAnimator);
        aTarget = GetAsPtr(fcaTarget->getTarget(), Animator);
    }
    aTarget->anim_switchRecording();
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
    update();
}

void BoxSingleWidget::switchBoxVisibleAction() {
    const auto target = mTarget->getTarget();
    if(!target) return;
    if(target->SWT_isBoundingBox()) {
        GetAsPtr(target, BoundingBox)->switchVisible();
    } else if(target->SWT_isPixmapEffect()) {
        GetAsPtr(target, PixmapEffect)->switchVisible();
    } else if(target->SWT_isPathEffect()) {
        GetAsPtr(target, PathEffect)->switchVisible();
    }
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
    update();
}

void BoxSingleWidget::switchBoxLockedAction() {
    GetAsPtr(mTarget->getTarget(), BoundingBox)->switchLocked();
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
    update();
}

void BoxSingleWidget::updateValueSlidersForQPointFAnimator() {
    if(!mTarget) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(!target->SWT_isQPointFAnimator() ||
        mTarget->contentVisible()) return;
    int nameRightX = getOptimalNameRightX();
    int slidersWidth = mValueSlider->minimumWidth() +
            mSecondValueSlider->minimumWidth() + MIN_WIDGET_HEIGHT;
    if(width() - nameRightX > slidersWidth) {
        const auto pt_target = GetAsPtr(target, QPointFAnimator);
        mValueSlider->setTarget(pt_target->getXAnimator());
        mValueSlider->show();
        mValueSlider->setNeighbouringSliderToTheRight(true);
        mSecondValueSlider->setTarget(pt_target->getYAnimator());
        mSecondValueSlider->show();
        mSecondValueSlider->setNeighbouringSliderToTheLeft(true);
    } else {
        clearAndHideValueAnimators();
    }
}

void BoxSingleWidget::clearColorButton() {
    mColorButton->setColorTarget(nullptr);
    mColorButton->hide();
}

void BoxSingleWidget::updateCompositionBoxVisible() {
    if(!mTarget) return;
    if(mCompositionModeVisible) {
        if(width() > 20*MIN_WIDGET_HEIGHT) {
            mCompositionModeCombo->show();
        } else {
            mCompositionModeCombo->hide();
        }
    }
    updateValueSlidersForQPointFAnimator();
}

void BoxSingleWidget::resizeEvent(QResizeEvent *) {
    updateCompositionBoxVisible();
}
