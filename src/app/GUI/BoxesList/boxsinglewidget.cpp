// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "boxsinglewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "GUI/ColorWidgets/colorsettingswidget.h"

#include "Boxes/containerbox.h"
#include "GUI/qrealanimatorvalueslider.h"
#include "boxscroller.h"
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
#include "Boxes/pathbox.h"
#include "canvas.h"

#include "Animators/SmartPath/smartpathcollection.h"

#include "typemenu.h"

QPixmap* BoxSingleWidget::VISIBLE_PIXMAP;
QPixmap* BoxSingleWidget::INVISIBLE_PIXMAP;
QPixmap* BoxSingleWidget::BOX_CHILDREN_VISIBLE;
QPixmap* BoxSingleWidget::BOX_CHILDREN_HIDDEN;
QPixmap* BoxSingleWidget::ANIMATOR_CHILDREN_VISIBLE;
QPixmap* BoxSingleWidget::ANIMATOR_CHILDREN_HIDDEN;
QPixmap* BoxSingleWidget::LOCKED_PIXMAP;
QPixmap* BoxSingleWidget::UNLOCKED_PIXMAP;
QPixmap* BoxSingleWidget::MUTED_PIXMAP;
QPixmap* BoxSingleWidget::UNMUTED_PIXMAP;
QPixmap* BoxSingleWidget::ANIMATOR_RECORDING;
QPixmap* BoxSingleWidget::ANIMATOR_NOT_RECORDING;
QPixmap* BoxSingleWidget::ANIMATOR_DESCENDANT_RECORDING;
QPixmap* BoxSingleWidget::C_PIXMAP;
QPixmap* BoxSingleWidget::G_PIXMAP;
QPixmap* BoxSingleWidget::CG_PIXMAP;
QPixmap* BoxSingleWidget::GRAPH_PROPERTY;

bool BoxSingleWidget::sStaticPixmapsLoaded = false;

#include "GUI/global.h"
#include "GUI/mainwindow.h"
#include "clipboardcontainer.h"
#include "Timeline/durationrectangle.h"
#include "GUI/coloranimatorbutton.h"
#include "canvas.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
#include "Sound/singlesound.h"

#include <QApplication>
#include <QDrag>
#include <QMenu>
#include <QInputDialog>

BoxSingleWidget::BoxSingleWidget(BoxScroller * const parent) :
    SingleWidget(parent), mParent(parent) {
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
            const auto caTarget = static_cast<ComplexAnimator*>(target);
            if(caTarget->anim_isRecording()) {
                return BoxSingleWidget::ANIMATOR_RECORDING;
            } else {
                if(caTarget->anim_isDescendantRecording()) {
                    return BoxSingleWidget::ANIMATOR_DESCENDANT_RECORDING;
                } else return BoxSingleWidget::ANIMATOR_NOT_RECORDING;
            }
        } else if(target->SWT_isAnimator()) {
            if(static_cast<Animator*>(target)->anim_isRecording()) {
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
        if(target->SWT_isBoundingBox() || target->SWT_isSingleSound()) {
            if(mTarget->contentVisible()) {
                return BoxSingleWidget::BOX_CHILDREN_VISIBLE;
            } else return BoxSingleWidget::BOX_CHILDREN_HIDDEN;
        } else {
            if(mTarget->contentVisible()) {
                return BoxSingleWidget::ANIMATOR_CHILDREN_VISIBLE;
            } else return BoxSingleWidget::ANIMATOR_CHILDREN_HIDDEN;
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
            if(static_cast<BoundingBox*>(target)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else if(target->SWT_isSound()) {
            if(static_cast<SingleSound*>(target)->isVisible()) {
                return BoxSingleWidget::UNMUTED_PIXMAP;
            } else return BoxSingleWidget::MUTED_PIXMAP;
        } else if(target->SWT_isRasterEffect()) {
            if(static_cast<RasterEffect*>(target)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else if(target->SWT_isPathEffect()) {
            if(static_cast<PathEffect*>(target)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else if(target->SWT_isGraphAnimator()) {
            const auto bsvt = static_cast<BoxScroller*>(mParent);
            const auto keysView = bsvt->getKeysView();
            if(keysView) return BoxSingleWidget::GRAPH_PROPERTY;
            return static_cast<QPixmap*>(nullptr);
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
            if(static_cast<BoundingBox*>(target)->isLocked()) {
                return BoxSingleWidget::LOCKED_PIXMAP;
            } else return BoxSingleWidget::UNLOCKED_PIXMAP;
        } else return static_cast<QPixmap*>(nullptr);
    });

    mMainLayout->addWidget(mLockedButton);
    connect(mLockedButton, &BoxesListActionButton::pressed,
            this, &BoxSingleWidget::switchBoxLockedAction);

    mHwSupportButton = new PixmapActionButton(this);
    mHwSupportButton->setPixmapChooser([this]() {
        if(!mTarget) return static_cast<QPixmap*>(nullptr);
        const auto target = mTarget->getTarget();
        if(target->SWT_isRasterEffect()) {
            const auto rEff = static_cast<RasterEffect*>(target);
            if(rEff->instanceHwSupport() == HardwareSupport::cpuOnly) {
                return BoxSingleWidget::C_PIXMAP;
            } else if(rEff->instanceHwSupport() == HardwareSupport::gpuOnly) {
                return BoxSingleWidget::G_PIXMAP;
            } else return BoxSingleWidget::CG_PIXMAP;
        } else return static_cast<QPixmap*>(nullptr);
    });

    mMainLayout->addWidget(mHwSupportButton);
    connect(mHwSupportButton, &BoxesListActionButton::pressed, this, [this]() {
        if(!mTarget) return;
        const auto target = mTarget->getTarget();
        if(!target->SWT_isRasterEffect()) return;
        const auto rEff = static_cast<RasterEffect*>(target);
        rEff->switchInstanceHwSupport();
        Document::sInstance->actionFinished();
    });

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
    connect(mPropertyComboBox, qOverload<int>(&QComboBox::activated),
            this, [this](const int id) {
        if(!mTarget) return;
        const auto target = mTarget->getTarget();
        if(!target->SWT_isComboBoxProperty()) return;
        const auto comboBoxProperty = static_cast<ComboBoxProperty*>(target);
        comboBoxProperty->setCurrentValue(id);
        Document::sInstance->actionFinished();
    });
    mMainLayout->addWidget(mPropertyComboBox);

    mBlendModeCombo = new QComboBox(this);
    mBlendModeCombo->setFocusPolicy(Qt::NoFocus);
    mMainLayout->addWidget(mBlendModeCombo);

    for(int modeId = int(SkBlendMode::kSrcOver);
        modeId <= int(SkBlendMode::kLastMode); modeId++) {
        const auto mode = static_cast<SkBlendMode>(modeId);
        if(mode == SkBlendMode::kSrcOut) continue;
        if(mode == SkBlendMode::kSrcATop) continue;
        if(mode == SkBlendMode::kDstATop) continue;
        if(mode == SkBlendMode::kModulate) continue;
        mBlendModeCombo->addItem(SkBlendMode_Name(mode), modeId);
    }

    mBlendModeCombo->insertSeparator(8);
    mBlendModeCombo->insertSeparator(19);
    connect(mBlendModeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setCompositionMode);
    mBlendModeCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    mPathBlendModeCombo = new QComboBox(this);
    mPathBlendModeCombo->setFocusPolicy(Qt::NoFocus);
    mMainLayout->addWidget(mPathBlendModeCombo);
    mPathBlendModeCombo->addItems(QStringList() << "Normal" <<
                                  "Add" << "Remove" << "Remove reverse" <<
                                  "Intersect" << "Exclude" << "Divide");
    connect(mPathBlendModeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setPathCompositionMode);
    mPathBlendModeCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    mFillTypeCombo = new QComboBox(this);
    mFillTypeCombo->setFocusPolicy(Qt::NoFocus);
    mMainLayout->addWidget(mFillTypeCombo);
    mFillTypeCombo->addItems(QStringList() << "Winding" << "Even-odd");
    connect(mFillTypeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setFillType);
    mFillTypeCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    mPropertyComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    mBoxTargetWidget = new BoxTargetWidget(this);
    mMainLayout->addWidget(mBoxTargetWidget);

    mCheckBox = new BoolPropertyWidget(this);
    mMainLayout->addWidget(mCheckBox);

    mMainLayout->addSpacing(MIN_WIDGET_DIM/2);

    hide();
}

void BoxSingleWidget::setCompositionMode(const int id) {
    if(!mTarget) return;
    const auto target = mTarget->getTarget();

    if(target->SWT_isBoundingBox()) {
        const auto boxTarget = static_cast<BoundingBox*>(target);
        const int modeId = mBlendModeCombo->itemData(id).toInt();
        const auto mode = static_cast<SkBlendMode>(modeId);
        boxTarget->setBlendModeSk(mode);
    }
    Document::sInstance->actionFinished();
}

void BoxSingleWidget::setPathCompositionMode(const int id) {
    if(!mTarget) return;
    const auto target = mTarget->getTarget();

    if(target->SWT_isSmartPathAnimator()) {
        const auto pAnim = static_cast<SmartPathAnimator*>(target);
        pAnim->setMode(static_cast<SmartPathAnimator::Mode>(id));
    }
    Document::sInstance->actionFinished();
}

void BoxSingleWidget::setFillType(const int id) {
    if(!mTarget) return;
    const auto target = mTarget->getTarget();

    if(target->SWT_isSmartPathCollection()) {
        const auto pAnim = static_cast<SmartPathCollection*>(target);
        pAnim->setFillType(static_cast<SkPath::FillType>(id));
    }
    Document::sInstance->actionFinished();
}

ColorAnimator *BoxSingleWidget::getColorTarget() const {
    const auto swt = mTarget->getTarget();
    ColorAnimator * color = nullptr;
    if(swt->SWT_isColorAnimator()) {
        color = static_cast<ColorAnimator*>(mTarget->getTarget());
    } else if(swt->SWT_isComplexAnimator()) {
        const auto ca = static_cast<ComplexAnimator*>(mTarget->getTarget());
        color = static_cast<ColorAnimator*>(ca->getPropertyForGUI());
    }
    return color;
}

void BoxSingleWidget::clearAndHideValueAnimators() {
    mValueSlider->clearTarget();
    mValueSlider->hide();
    mSecondValueSlider->clearTarget();
    mSecondValueSlider->hide();
}

void BoxSingleWidget::setTargetAbstraction(SWT_Abstraction *abs) {
    if(mTarget) {
        const auto oldTarget = mTarget->getTarget();
        disconnect(oldTarget, nullptr, this, nullptr);
    }
    SingleWidget::setTargetAbstraction(abs);
    if(!abs) return;
    auto target = abs->getTarget();
    connect(target, &SingleWidgetTarget::SWT_changedDisabled,
            this, qOverload<>(&QWidget::update));
    mContentButton->setVisible(target->SWT_isComplexAnimator());
    mRecordButton->setVisible(target->SWT_isAnimator() &&
                              !target->SWT_isBoundingBox() &&
                              !target->SWT_isSingleSound());
    mVisibleButton->setVisible(target->SWT_isBoundingBox() ||
                               target->SWT_isSound() ||
                               target->SWT_isPathEffect() ||
                               target->SWT_isRasterEffect() ||
                               target->SWT_isGraphAnimator());
    mLockedButton->setVisible(target->SWT_isBoundingBox());
    mHwSupportButton->setVisible(target->SWT_isRasterEffect());

    mBoxTargetWidget->setVisible(target->SWT_isBoxTargetProperty());
    mCheckBox->setVisible(target->SWT_isBoolProperty() ||
                          target->SWT_isBoolPropertyContainer());

    mPropertyComboBox->setVisible(target->SWT_isComboBoxProperty());

    mPathBlendModeVisible = false;
    mBlendModeVisible = false;
    mFillTypeVisible = false;
    mSelected = false;

    mColorButton->setColorTarget(nullptr);
    mValueSlider->clearTarget();
    mSecondValueSlider->clearTarget();

    bool valueSliderVisible = false;
    bool secondValueSliderVisible = false;
    bool colorButtonVisible = false;

    if(target->SWT_isBoundingBox()) {
        const auto boxPtr = static_cast<BoundingBox*>(target);

        mBlendModeVisible = true;
        mBlendModeCombo->setCurrentText(SkBlendMode_Name(boxPtr->getBlendMode()));
        mBlendModeCombo->setEnabled(!target->SWT_isGroupBox());
        connect(boxPtr, &BoundingBox::blendModeChanged,
                this, [this](const SkBlendMode mode) {
            mBlendModeCombo->setCurrentText(SkBlendMode_Name(mode));
        });
    } else if(target->SWT_isSingleSound()) {
    } else if(target->SWT_isBoolProperty()) {
        const auto bTarget = static_cast<BoolProperty*>(target);
        mCheckBox->setTarget(bTarget);
        connect(bTarget, &BoolProperty::valueChanged,
                this, [this]() { mCheckBox->update(); });
    } else if(target->SWT_isBoolPropertyContainer()) {
        const auto bTarget = static_cast<BoolPropertyContainer*>(target);
        mCheckBox->setTarget(bTarget);
        connect(bTarget, &BoolPropertyContainer::valueChanged,
                this, [this]() { mCheckBox->update(); });
    } else if(target->SWT_isComboBoxProperty()) {
        const auto comboBoxProperty = static_cast<ComboBoxProperty*>(target);
        mPropertyComboBox->clear();
        mPropertyComboBox->addItems(comboBoxProperty->getValueNames());
        mPropertyComboBox->setCurrentIndex(comboBoxProperty->getCurrentValue());
        connect(comboBoxProperty, &ComboBoxProperty::valueChanged,
                this, [this](const int id) {
            mPropertyComboBox->setCurrentIndex(id);
        });
    } else if(target->SWT_isIntProperty() || target->SWT_isQrealAnimator()) {
        if(target->SWT_isQrealAnimator())
            mValueSlider->setTarget(static_cast<QrealAnimator*>(target));
        else mValueSlider->setTarget(static_cast<IntProperty*>(target));
        valueSliderVisible = true;
        mValueSlider->setNeighbouringSliderToTheRight(false);
    } else if(target->SWT_isComplexAnimator()) {
        if(target->SWT_isColorAnimator()) {
            colorButtonVisible = true;
            mColorButton->setColorTarget(static_cast<ColorAnimator*>(target));
        } else if(target->SWT_isSmartPathCollection()) {
            const auto coll = static_cast<SmartPathCollection*>(target);
            mFillTypeVisible = true;
            mFillTypeCombo->setCurrentIndex(coll->getFillType());
            connect(coll, &SmartPathCollection::fillTypeChanged,
                    this, [this](const SkPath::FillType type) {
                mFillTypeCombo->setCurrentIndex(type);
            });
        }
        if(target->SWT_isComplexAnimator() && !abs->contentVisible()) {
            if(target->SWT_isQPointFAnimator()) {
                updateValueSlidersForQPointFAnimator();
                valueSliderVisible = mValueSlider->isVisible();
                secondValueSliderVisible = mSecondValueSlider->isVisible();
            } else {
                const auto ca_target = static_cast<ComplexAnimator*>(target);
                Property * const guiProp = ca_target->getPropertyForGUI();
                if(guiProp) {
                    if(guiProp->SWT_isQrealAnimator()) {
                        valueSliderVisible = true;
                        mValueSlider->setTarget(static_cast<QrealAnimator*>(guiProp));
                        mValueSlider->setNeighbouringSliderToTheRight(false);
                        mSecondValueSlider->clearTarget();
                    } else if(guiProp->SWT_isColorAnimator()) {
                        mColorButton->setColorTarget(static_cast<ColorAnimator*>(guiProp));
                        colorButtonVisible = true;
                    }
                }
            }
        }
    } else if(target->SWT_isBoxTargetProperty()) {
        mBoxTargetWidget->setTargetProperty(static_cast<BoxTargetProperty*>(target));
    } else if(target->SWT_isSmartPathAnimator()) {
        const auto path = static_cast<SmartPathAnimator*>(target);
        mPathBlendModeVisible = true;
        mPathBlendModeCombo->setCurrentIndex(path->getMode());
        connect(path, &SmartPathAnimator::pathBlendModeChagned,
                this, [this](const SmartPathAnimator::Mode mode) {
            mPathBlendModeCombo->setCurrentIndex(mode);
        });
    }

    if(target->SWT_isAnimator()) {
        connect(static_cast<Animator*>(target), &Animator::anim_isRecordingChanged,
                this, [this]() { mRecordButton->update(); });
    }
    if(target->SWT_isPathEffect() || target->SWT_isRasterEffect()) {
        const auto effTarget = static_cast<eEffect*>(target);
        connect(effTarget, &eEffect::effectVisibilityChanged,
                this, [this]() { mVisibleButton->update(); });
    }
    if(target->SWT_isBoundingBox() || target->SWT_isSound()) {
        const auto ptr = static_cast<eBoxOrSound*>(target);
        connect(ptr, &eBoxOrSound::visibilityChanged,
                this, [this]() { mVisibleButton->update(); });
        connect(ptr, &eBoxOrSound::selectionChanged,
                this, qOverload<>(&QWidget::update));
        connect(ptr, &eBoxOrSound::lockedChanged,
                this, [this]() { mLockedButton->update(); });
    }
    if(!target->SWT_isBoundingBox() && !target->SWT_isSingleSound()) {
        const auto prop = static_cast<Property*>(target);
        connect(prop, &Property::prp_selectionChanged,
                this, qOverload<>(&QWidget::update));
    }

    mValueSlider->setVisible(valueSliderVisible);
    mSecondValueSlider->setVisible(secondValueSliderVisible);
    mColorButton->setVisible(colorButtonVisible);

    updateCompositionBoxVisible();
    updatePathCompositionBoxVisible();
    updateFillTypeBoxVisible();
}

void BoxSingleWidget::loadStaticPixmaps() {
    if(sStaticPixmapsLoaded) return;
    const auto iconsDir = eSettings::sIconsDir();
    VISIBLE_PIXMAP = new QPixmap(iconsDir + "/visible.png");
    INVISIBLE_PIXMAP = new QPixmap(iconsDir + "/hidden.png");
    BOX_CHILDREN_VISIBLE = new QPixmap(iconsDir + "/childrenVisible.png");
    BOX_CHILDREN_HIDDEN = new QPixmap(iconsDir + "/childrenHidden.png");
    ANIMATOR_CHILDREN_VISIBLE = new QPixmap(iconsDir + "/childrenVisibleSmall.png");
    ANIMATOR_CHILDREN_HIDDEN = new QPixmap(iconsDir + "/childrenHiddenSmall.png");
    LOCKED_PIXMAP = new QPixmap(iconsDir + "/locked.png");
    UNLOCKED_PIXMAP = new QPixmap(iconsDir + "/unlocked.png");
    MUTED_PIXMAP = new QPixmap(iconsDir + "/muted.png");
    UNMUTED_PIXMAP = new QPixmap(iconsDir + "/unmuted.png");
    ANIMATOR_RECORDING = new QPixmap(iconsDir + "/recording.png");
    ANIMATOR_NOT_RECORDING = new QPixmap(iconsDir + "/notRecording.png");
    ANIMATOR_DESCENDANT_RECORDING = new QPixmap(iconsDir + "/childRecording.png");
    C_PIXMAP = new QPixmap(iconsDir + "/c.png");
    G_PIXMAP = new QPixmap(iconsDir + "/g.png");
    CG_PIXMAP = new QPixmap(iconsDir + "/cg.png");
    GRAPH_PROPERTY = new QPixmap(iconsDir + "/graphProperty.png");
    sStaticPixmapsLoaded = true;
}

void BoxSingleWidget::clearStaticPixmaps() {
    if(!sStaticPixmapsLoaded) return;
    delete VISIBLE_PIXMAP;
    delete INVISIBLE_PIXMAP;
    delete BOX_CHILDREN_VISIBLE;
    delete BOX_CHILDREN_HIDDEN;
    delete ANIMATOR_CHILDREN_VISIBLE;
    delete ANIMATOR_CHILDREN_HIDDEN;
    delete LOCKED_PIXMAP;
    delete UNLOCKED_PIXMAP;
    delete MUTED_PIXMAP;
    delete UNMUTED_PIXMAP;
    delete ANIMATOR_RECORDING;
    delete ANIMATOR_NOT_RECORDING;
    delete ANIMATOR_DESCENDANT_RECORDING;
}

void BoxSingleWidget::mousePressEvent(QMouseEvent *event) {
    if(!mTarget) return;
    if(isTargetDisabled()) return;
    const auto target = mTarget->getTarget();
    if(event->button() == Qt::RightButton) {
        setSelected(true);
        QMenu menu(this);

        if(target->SWT_isProperty()) {
            const auto pTarget = static_cast<Property*>(target);
            const bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
            if(target->SWT_isBoundingBox()) {
                const auto box = static_cast<BoundingBox*>(target);
                if(!box->isSelected()) box->selectionChangeTriggered(shiftPressed);
            } else {
                if(!pTarget->prp_isSelected()) pTarget->prp_selectionChangeTriggered(shiftPressed);
            }
            PropertyMenu pMenu(&menu, mParent->currentScene(), MainWindow::sGetInstance());
            pTarget->setupTreeViewMenu(&pMenu);
        }
        menu.exec(event->globalPos());
        setSelected(false);
    } else {
        mDragPressPos = event->pos().x() > mFillWidget->x();
        mDragStartPos = event->pos();
    }
    Document::sInstance->actionFinished();
}

bool BoxSingleWidget::isTargetDisabled() {
    if(!mTarget) return true;
    return mTarget->getTarget()->SWT_isDisabled();
}

void BoxSingleWidget::mouseMoveEvent(QMouseEvent *event) {
    if(!mDragPressPos) return;
    if(!(event->buttons() & Qt::LeftButton)) return;
    if(isTargetDisabled()) return;
    const auto dist = (event->pos() - mDragStartPos).manhattanLength();
    if(dist < QApplication::startDragDistance()) return;
    const auto drag = new QDrag(this);
    {
        const auto prop = static_cast<Property*>(mTarget->getTarget());
        const QString name = prop->prp_getName();
        const int nameWidth = QApplication::fontMetrics().width(name);
        QPixmap pixmap(mFillWidget->x() + nameWidth + MIN_WIDGET_DIM, height());
        render(&pixmap);
        drag->setPixmap(pixmap);
    }
    connect(drag, &QDrag::destroyed, this, &BoxSingleWidget::clearSelected);

    const auto mimeData = mTarget->getTarget()->SWT_createMimeData();
    if(!mimeData) return;
    setSelected(true);
    drag->setMimeData(mimeData);

    drag->installEventFilter(MainWindow::sGetInstance());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void BoxSingleWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(isTargetDisabled()) return;
    setSelected(false);
    if(pointToLen(event->pos() - mDragStartPos) > MIN_WIDGET_DIM/2) return;
    const bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
    const auto target = mTarget->getTarget();
    if(target->SWT_isBoundingBox() && !target->SWT_isCanvas()) {
        auto boxTarget = static_cast<BoundingBox*>(target);
        boxTarget->selectionChangeTriggered(shiftPressed);
        Document::sInstance->actionFinished();
    } else if(target->SWT_isProperty()) {
        const auto pTarget = static_cast<Property*>(target);
        pTarget->prp_selectionChangeTriggered(shiftPressed);
    }
}

void BoxSingleWidget::mouseDoubleClickEvent(QMouseEvent *e) {
    if(isTargetDisabled()) return;
    if(e->modifiers() & Qt::ShiftModifier) {
        //mousePressEvent(e);
    } else Document::sInstance->actionFinished();
}

void BoxSingleWidget::drawTimelineControls(QPainter * const p,
                               const qreal pixelsPerFrame,
                               const FrameRange &viewedFrames) {
    if(isHidden() || !mTarget) return;
    const auto target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        const auto anim_target = static_cast<Animator*>(target);
        anim_target->drawTimelineControls(p, pixelsPerFrame, viewedFrames,
                                          MIN_WIDGET_DIM);
    }
}

Key* BoxSingleWidget::getKeyAtPos(const int pressX,
                                  const qreal pixelsPerFrame,
                                  const int minViewedFrame) {
    if(isHidden() || !mTarget) return nullptr;
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

TimelineMovable* BoxSingleWidget::getRectangleMovableAtPos(
                            const int pressX,
                            const qreal pixelsPerFrame,
                            const int minViewedFrame) {
    if(isHidden() || !mTarget) return nullptr;
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
                                    const qreal pixelsPerFrame,
                                    QList<Key*>& listKeys) {
    if(isHidden() || !mTarget) return;
    const auto target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        const auto anim_target = static_cast<Animator*>(target);

        anim_target->anim_getKeysInRect(selectionRect, pixelsPerFrame,
                                       listKeys, KEY_RECT_SIZE);
    }
}

void BoxSingleWidget::paintEvent(QPaintEvent *) {
    if(!mTarget) return;
    QPainter p(this);
    auto target = mTarget->getTarget();
    if(target->SWT_isDisabled()) p.setOpacity(.5);

    int nameX = mFillWidget->x();
    const QString name = static_cast<Property*>(target)->prp_getName();
    if(!target->SWT_isBoundingBox() && !target->SWT_isSingleSound()) {
        const auto prop = static_cast<Property*>(target);
        if(prop->prp_isSelected()) {
            p.fillRect(mFillWidget->geometry(), QColor(0, 0, 0, 55));
        }
    }
    if(target->SWT_isBoundingBox() || target->SWT_isSingleSound()) {
        const auto bsTarget = static_cast<eBoxOrSound*>(target);

        nameX += MIN_WIDGET_DIM/4;

        const bool ss = target->SWT_isSingleSound();
        if(ss) p.fillRect(rect(), QColor(0, 125, 255, 50));
        else   p.fillRect(rect(), QColor(0, 0, 0, 50));

        if(bsTarget->isSelected()) {
            if(ss) p.fillRect(mFillWidget->geometry(), QColor(125, 200, 255));
            else   p.fillRect(mFillWidget->geometry(), QColor(180, 180, 180));
            p.setPen(Qt::black);
        } else {
            p.setPen(Qt::white);
        }
    } else if(!target->SWT_isComplexAnimator()) {
        if(target->SWT_isGraphAnimator()) {
            const auto graphAnim = static_cast<GraphAnimator*>(target);
            const auto bswvp = static_cast<BoxScroller*>(mParent);
            const auto keysView = bswvp->getKeysView();
            if(keysView) {
                const int id = keysView->graphGetAnimatorId(graphAnim);
                if(id >= 0) {
                    const auto color = keysView->sGetAnimatorColor(id);
                    const QRect visRect(mVisibleButton->pos(),
                                        GRAPH_PROPERTY->size());
                    const int adj = qRound(4*qreal(GRAPH_PROPERTY->width())/20);
                    p.fillRect(visRect.adjusted(adj, adj, -adj, -adj), color);
                }
            }
        } else nameX += MIN_WIDGET_DIM;

        p.setPen(Qt::white);
    } else { //if(target->SWT_isComplexAnimator()) {
        p.setPen(Qt::white);
    }

    const QRect textRect(nameX, 0, width() - nameX - MIN_WIDGET_DIM, MIN_WIDGET_DIM);
    p.drawText(textRect, name, QTextOption(Qt::AlignVCenter));
    if(mSelected) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::lightGray));
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
    p.end();
}

void BoxSingleWidget::switchContentVisibleAction() {
    if(!mTarget) return;
    mTarget->switchContentVisible();
    Document::sInstance->actionFinished();
    //mParent->callUpdaters();
}

void BoxSingleWidget::switchRecordingAction() {
    if(!mTarget) return;
    const auto target = mTarget->getTarget();
    if(!target) return;
    if(!target->SWT_isAnimator()) return;
    auto aTarget = static_cast<Animator*>(target);
    aTarget->anim_switchRecording();
    Document::sInstance->actionFinished();
    update();
}

void BoxSingleWidget::switchBoxVisibleAction() {
    if(!mTarget) return;
    const auto target = mTarget->getTarget();
    if(!target) return;
    if(target->SWT_isBoundingBox() || target->SWT_isSound()) {
        static_cast<eBoxOrSound*>(target)->switchVisible();
    } else if(target->SWT_isRasterEffect()) {
        static_cast<RasterEffect*>(target)->switchVisible();
    } else if(target->SWT_isPathEffect()) {
        static_cast<PathEffect*>(target)->switchVisible();
    } else if(target->SWT_isGraphAnimator()) {
        const auto bsvt = static_cast<BoxScroller*>(mParent);
        const auto keysView = bsvt->getKeysView();
        if(keysView) {
            const auto animTarget = static_cast<GraphAnimator*>(target);
            if(keysView->graphGetAnimatorId(animTarget) != -1) {
                keysView->graphRemoveViewedAnimator(animTarget);
            } else {
                keysView->graphAddViewedAnimator(animTarget);
            }
            Document::sInstance->actionFinished();
        }
    }
    Document::sInstance->actionFinished();
    update();
}

void BoxSingleWidget::switchBoxLockedAction() {
    if(!mTarget) return;
    static_cast<BoundingBox*>(mTarget->getTarget())->switchLocked();
    Document::sInstance->actionFinished();
    update();
}

void BoxSingleWidget::updateValueSlidersForQPointFAnimator() {
    if(!mTarget) return;
    const auto target = mTarget->getTarget();
    if(!target->SWT_isQPointFAnimator() || mTarget->contentVisible()) return;
    if(width() - mFillWidget->x() > 10*MIN_WIDGET_DIM) {
        const auto pt_target = static_cast<QPointFAnimator*>(target);
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

void BoxSingleWidget::updatePathCompositionBoxVisible() {
    if(!mTarget) return;
    if(mPathBlendModeVisible && width() - mFillWidget->x() > 8*MIN_WIDGET_DIM) {
        mPathBlendModeCombo->show();
    } else mPathBlendModeCombo->hide();
}

void BoxSingleWidget::updateCompositionBoxVisible() {
    if(!mTarget) return;
    if(mBlendModeVisible && width() - mFillWidget->x() > 10*MIN_WIDGET_DIM) {
        mBlendModeCombo->show();
    } else mBlendModeCombo->hide();
}

void BoxSingleWidget::updateFillTypeBoxVisible() {
    if(!mTarget) return;
    if(mFillTypeVisible && width() - mFillWidget->x() > 8*MIN_WIDGET_DIM) {
        mFillTypeCombo->show();
    } else mFillTypeCombo->hide();
}

void BoxSingleWidget::resizeEvent(QResizeEvent *) {
    updateCompositionBoxVisible();
    updatePathCompositionBoxVisible();
    updateFillTypeBoxVisible();
    updateValueSlidersForQPointFAnimator();
}
