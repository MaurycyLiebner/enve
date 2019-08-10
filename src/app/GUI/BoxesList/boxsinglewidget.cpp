#include "boxsinglewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "GUI/ColorWidgets/colorsettingswidget.h"

#include "Boxes/containerbox.h"
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
QPixmap* BoxSingleWidget::ANIMATOR_RECORDING;
QPixmap* BoxSingleWidget::ANIMATOR_NOT_RECORDING;
QPixmap* BoxSingleWidget::ANIMATOR_DESCENDANT_RECORDING;
bool BoxSingleWidget::sStaticPixmapsLoaded = false;

#include "GUI/global.h"
#include "GUI/mainwindow.h"
#include "clipboardcontainer.h"
#include "Timeline/durationrectangle.h"
#include "GUI/coloranimatorbutton.h"
#include "canvas.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"

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
        if(target->SWT_isBoundingBox()) {
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
        } else if(target->SWT_isRasterEffect()) {
            if(static_cast<RasterEffect*>(target)->isVisible()) {
                return BoxSingleWidget::VISIBLE_PIXMAP;
            } else return BoxSingleWidget::INVISIBLE_PIXMAP;
        } else if(target->SWT_isPathEffect()) {
            if(static_cast<PathEffect*>(target)->isVisible()) {
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
            if(static_cast<BoundingBox*>(target)->isLocked()) {
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

    mBlendModeCombo = new QComboBox(this);
    mMainLayout->addWidget(mBlendModeCombo);

//    mBlendModeCombo->addItems(QStringList() <<
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
    mBlendModeCombo->addItems(QStringList() <<
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
    mBlendModeCombo->insertSeparator(10);
    mBlendModeCombo->insertSeparator(22);
    connect(mBlendModeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setCompositionMode);
    mBlendModeCombo->setSizePolicy(QSizePolicy::Maximum,
                    mBlendModeCombo->sizePolicy().horizontalPolicy());

    mPathBlendModeCombo = new QComboBox(this);
    mMainLayout->addWidget(mPathBlendModeCombo);
    mPathBlendModeCombo->addItems(QStringList() << "Normal" <<
                                  "Add" << "Remove" << "Remove reverse" <<
                                  "Intersect" << "Exclude" << "Divide");
    connect(mPathBlendModeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setPathCompositionMode);
    mPathBlendModeCombo->setSizePolicy(QSizePolicy::Maximum,
                    mPathBlendModeCombo->sizePolicy().horizontalPolicy());

    mFillTypeCombo = new QComboBox(this);
    mMainLayout->addWidget(mFillTypeCombo);
    mFillTypeCombo->addItems(QStringList() << "Winding" << "Even-odd");
    connect(mFillTypeCombo, qOverload<int>(&QComboBox::activated),
            this, &BoxSingleWidget::setFillType);
    mFillTypeCombo->setSizePolicy(QSizePolicy::Maximum,
                    mFillTypeCombo->sizePolicy().horizontalPolicy());

    mPropertyComboBox->setSizePolicy(QSizePolicy::Maximum,
                                     mPropertyComboBox->sizePolicy().horizontalPolicy());
    mBoxTargetWidget = new BoxTargetWidget(this);
    mMainLayout->addWidget(mBoxTargetWidget);

    mCheckBox = new BoolPropertyWidget(this);
    mMainLayout->addWidget(mCheckBox);

    mMainLayout->addSpacing(MIN_WIDGET_DIM/2);

    hide();
}

SkBlendMode idToBlendModeSk(const int id) {
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

void BoxSingleWidget::setCompositionMode(const int id) {
    const auto target = mTarget->getTarget();

    if(target->SWT_isBoundingBox()) {
        const auto boxTarget = static_cast<BoundingBox*>(target);
        boxTarget->setBlendModeSk(idToBlendModeSk(id));
    }
    Document::sInstance->actionFinished();
}

void BoxSingleWidget::setPathCompositionMode(const int id) {
    const auto target = mTarget->getTarget();

    if(target->SWT_isSmartPathAnimator()) {
        const auto pAnim = static_cast<SmartPathAnimator*>(target);
        pAnim->setMode(static_cast<SmartPathAnimator::Mode>(id));
    }
    Document::sInstance->actionFinished();
}

void BoxSingleWidget::setFillType(const int id) {
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
    SingleWidget::setTargetAbstraction(abs);
    auto target = abs->getTarget();

    mContentButton->setVisible(target->SWT_isComplexAnimator());
    mRecordButton->setVisible(target->SWT_isAnimator());
    mVisibleButton->setVisible(target->SWT_isBoundingBox() ||
                               target->SWT_isPathEffect() ||
                               target->SWT_isRasterEffect());
    mLockedButton->setVisible(target->SWT_isBoundingBox());
    mRecordButton->show();

    mFillTypeCombo->hide();
    mBlendModeCombo->hide();
    mPathBlendModeCombo->hide();
    mPropertyComboBox->hide();

    mPathBlendModeVisible = false;
    mBlendModeVisible = false;
    mFillTypeVisible = false;

    mBoxTargetWidget->hide();
    mCheckBox->hide();

    clearColorButton();
    clearAndHideValueAnimators();

    if(target->SWT_isBoundingBox()) {
        mRecordButton->hide();
        const auto boxPtr = static_cast<BoundingBox*>(target);

        mBlendModeVisible = true;
        mBlendModeCombo->setCurrentIndex(
            blendModeToIntSk(boxPtr->getBlendMode()));
        mBlendModeCombo->setEnabled(!target->SWT_isGroupBox());
        updateCompositionBoxVisible();
    } else if(target->SWT_isBoolProperty()) {
        mCheckBox->show();
        mCheckBox->setTarget(static_cast<BoolProperty*>(target));
    } else if(target->SWT_isBoolPropertyContainer()) {
        mCheckBox->show();
        mCheckBox->setTarget(static_cast<BoolPropertyContainer*>(target));
    } else if(target->SWT_isComboBoxProperty()) {
        disconnect(mPropertyComboBox, nullptr, nullptr, nullptr);
        if(mLastComboBoxProperty.data() != nullptr) {
            disconnect(mLastComboBoxProperty.data(), nullptr,
                       mPropertyComboBox, nullptr);
        }
        auto comboBoxProperty = static_cast<ComboBoxProperty*>(target);
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
                Document::sInstance,
                &Document::actionFinished);
    } else if(target->SWT_isIntProperty() || target->SWT_isQrealAnimator()) {
        if(target->SWT_isQrealAnimator())
            mValueSlider->setTarget(static_cast<QrealAnimator*>(target));
        else mValueSlider->setTarget(static_cast<IntProperty*>(target));
        mValueSlider->show();
        mValueSlider->setNeighbouringSliderToTheRight(false);
    } else if(target->SWT_isComplexAnimator()) {
        if(target->SWT_isColorAnimator()) {
            mColorButton->setColorTarget(static_cast<ColorAnimator*>(target));
            mColorButton->show();
        } else if(target->SWT_isSmartPathCollection()) {
            const auto coll = static_cast<SmartPathCollection*>(target);
            mFillTypeVisible = true;
            mFillTypeCombo->setCurrentIndex(coll->getFillType());
            updateFillTypeBoxVisible();
        }
        if(target->SWT_isComplexAnimator() && !abs->contentVisible()) {
            if(target->SWT_isQPointFAnimator()) {
                updateValueSlidersForQPointFAnimator();
            } else {
                const auto ca_target = static_cast<ComplexAnimator*>(target);
                Property * const guiProp = ca_target->getPropertyForGUI();
                if(guiProp) {
                    if(guiProp->SWT_isQrealAnimator()) {
                        mValueSlider->setTarget(static_cast<QrealAnimator*>(guiProp));
                        mValueSlider->show();
                        mValueSlider->setNeighbouringSliderToTheRight(false);
                        mSecondValueSlider->hide();
                        mSecondValueSlider->clearTarget();
                    } else if(guiProp->SWT_isColorAnimator()) {
                        mColorButton->setColorTarget(static_cast<ColorAnimator*>(guiProp));
                        mColorButton->show();
                    }
                }
            }
        }
    } else if(target->SWT_isBoxTargetProperty()) {
        mBoxTargetWidget->show();
        mBoxTargetWidget->setTargetProperty(
                    static_cast<BoxTargetProperty*>(target));
    } else if(target->SWT_isSmartPathAnimator()) {
        mPathBlendModeVisible = true;
        updatePathCompositionBoxVisible();
    }
}

void BoxSingleWidget::loadStaticPixmaps() {
    if(sStaticPixmapsLoaded) return;
    const auto iconsDir = EnveSettings::sIconsDir();
    VISIBLE_PIXMAP = new QPixmap(iconsDir + "/visible.png");
    INVISIBLE_PIXMAP = new QPixmap(iconsDir + "/hidden.png");
    BOX_CHILDREN_VISIBLE = new QPixmap(iconsDir + "/childrenVisible.png");
    BOX_CHILDREN_HIDDEN = new QPixmap(iconsDir + "/childrenHidden.png");
    ANIMATOR_CHILDREN_VISIBLE = new QPixmap(iconsDir + "/childrenVisibleSmall.png");
    ANIMATOR_CHILDREN_HIDDEN = new QPixmap(iconsDir + "/childrenHiddenSmall.png");
    LOCKED_PIXMAP = new QPixmap(iconsDir + "/locked.png");
    UNLOCKED_PIXMAP = new QPixmap(iconsDir + "/unlocked.png");
    ANIMATOR_RECORDING = new QPixmap(iconsDir + "/recording.png");
    ANIMATOR_NOT_RECORDING = new QPixmap(iconsDir + "/notRecording.png");
    ANIMATOR_DESCENDANT_RECORDING = new QPixmap(iconsDir + "/childRecording.png");
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
    delete ANIMATOR_RECORDING;
    delete ANIMATOR_NOT_RECORDING;
    delete ANIMATOR_DESCENDANT_RECORDING;
}

void BoxSingleWidget::mousePressEvent(QMouseEvent *event) {
    if(isTargetDisabled()) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(event->button() == Qt::RightButton) {
        setSelected(true);
        QMenu menu(this);

        if(target->SWT_isProperty()) {
            if(target->SWT_isBoundingBox()) {
                const auto box = static_cast<BoundingBox*>(target);
                const bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
                if(!box->isSelected()) box->selectionChangeTriggered(shiftPressed);
            }
            const auto pTarget = static_cast<Property*>(target);
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
    connect(drag, &QDrag::destroyed,
            this, &BoxSingleWidget::clearSelected);

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
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isBoundingBox() && !target->SWT_isCanvas()) {
        auto boxTarget = static_cast<BoundingBox*>(target);
        boxTarget->selectionChangeTriggered(event->modifiers() &
                                            Qt::ShiftModifier);
        Document::sInstance->actionFinished();
    } else if(target->SWT_isGraphAnimator()) {
        const auto animTarget = static_cast<GraphAnimator*>(target);
        const auto bsvt = static_cast<BoxScroller*>(mParent);
        KeysView * const keysView = bsvt->getKeysView();
        if(keysView) {
            if(keysView->graphGetAnimatorId(animTarget) != -1) {
                keysView->graphRemoveViewedAnimator(animTarget);
            } else {
                keysView->graphAddViewedAnimator(animTarget);
            }
            Document::sInstance->actionFinished();
        }
    }
}

void BoxSingleWidget::mouseDoubleClickEvent(QMouseEvent *e) {
    if(isTargetDisabled()) return;
    if(e->modifiers() & Qt::ShiftModifier) {
        //mousePressEvent(e);
    } else Document::sInstance->actionFinished();
}
void BoxSingleWidget::drawKeys(QPainter * const p,
                               const qreal pixelsPerFrame,
                               const FrameRange &viewedFrames) {
    if(isHidden()) return;
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
                            const int pressX,
                            const qreal pixelsPerFrame,
                            const int minViewedFrame) {
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
                                    const qreal pixelsPerFrame,
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

    QFontMetrics fm = QFontMetrics(QFont());
    QString name;
    if(target->SWT_isProperty()) {
        name = static_cast<Property*>(target)->prp_getName();
    }
    int nameX = mFillWidget->x();
    //return nameX;
    if(target->SWT_isBoundingBox()) {
        nameX += MIN_WIDGET_DIM/4;
    } else if(target->SWT_isQrealAnimator()) {
        nameX += MIN_WIDGET_DIM;
    } else if(target->SWT_isBoxTargetProperty()) {
        nameX += 2*MIN_WIDGET_DIM;
    } else {//if(target->SWT_isBoolProperty()) {
        nameX += 2*MIN_WIDGET_DIM;
    }
    return nameX + fm.width(name);
}

void BoxSingleWidget::paintEvent(QPaintEvent *) {
    if(!mTarget) return;
    QPainter p(this);
    auto target = mTarget->getTarget();
    if(target->SWT_isDisabled()) p.setOpacity(.5);

    int nameX = mFillWidget->x();
    QString name;
    if(target->SWT_isBoundingBox()) {
        const auto bb_target = static_cast<BoundingBox*>(target);

        nameX += MIN_WIDGET_DIM/4;
        name = bb_target->prp_getName();

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
            const auto graphAnim = static_cast<GraphAnimator*>(target);
            const auto bswvp = static_cast<BoxScroller*>(mParent);
            const auto keysView = bswvp->getKeysView();
            if(keysView) {
                const int id = keysView->graphGetAnimatorId(graphAnim);
                if(id >= 0) {
                    const auto color = keysView->sGetAnimatorColor(id);
                    p.fillRect(nameX + MIN_WIDGET_DIM/4, MIN_WIDGET_DIM/4,
                               MIN_WIDGET_DIM/2, MIN_WIDGET_DIM/2, color);
                }
            }
        }
        name = propTarget->prp_getName();
        nameX += MIN_WIDGET_DIM;

        p.setPen(Qt::white);
    } else { //if(target->SWT_isComplexAnimator()) {
        ComplexAnimator *caTarget = static_cast<ComplexAnimator*>(target);
        name = caTarget->prp_getName();

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
    mTarget->switchContentVisible();
    Document::sInstance->actionFinished();
    //mParent->callUpdaters();
}

void BoxSingleWidget::switchRecordingAction() {
    const auto target = mTarget->getTarget();
    if(!target) return;
    if(!target->SWT_isAnimator()) return;
    auto aTarget = static_cast<Animator*>(target);
    aTarget->anim_switchRecording();
    Document::sInstance->actionFinished();
    update();
}

void BoxSingleWidget::switchBoxVisibleAction() {
    const auto target = mTarget->getTarget();
    if(!target) return;
    if(target->SWT_isBoundingBox()) {
        static_cast<BoundingBox*>(target)->switchVisible();
    } else if(target->SWT_isRasterEffect()) {
        static_cast<RasterEffect*>(target)->switchVisible();
    } else if(target->SWT_isPathEffect()) {
        static_cast<PathEffect*>(target)->switchVisible();
    }
    Document::sInstance->actionFinished();
    update();
}

void BoxSingleWidget::switchBoxLockedAction() {
    static_cast<BoundingBox*>(mTarget->getTarget())->switchLocked();
    Document::sInstance->actionFinished();
    update();
}

void BoxSingleWidget::updateValueSlidersForQPointFAnimator() {
    if(!mTarget) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(!target->SWT_isQPointFAnimator() ||
        mTarget->contentVisible()) return;
    int nameRightX = getOptimalNameRightX();
    int slidersWidth = mValueSlider->minimumWidth() +
            mSecondValueSlider->minimumWidth() + MIN_WIDGET_DIM;
    if(width() - nameRightX > slidersWidth) {
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

void BoxSingleWidget::clearColorButton() {
    mColorButton->setColorTarget(nullptr);
    mColorButton->hide();
}

void BoxSingleWidget::updatePathCompositionBoxVisible() {
    if(!mTarget) return;
    if(mPathBlendModeVisible) {
        if(width() > 15*MIN_WIDGET_DIM) mPathBlendModeCombo->show();
        else mPathBlendModeCombo->hide();
    }
}

void BoxSingleWidget::updateCompositionBoxVisible() {
    if(!mTarget) return;
    if(mBlendModeVisible) {
        if(width() > 15*MIN_WIDGET_DIM) mBlendModeCombo->show();
        else mBlendModeCombo->hide();
    }
}

void BoxSingleWidget::updateFillTypeBoxVisible() {
    if(!mTarget) return;
    if(mFillTypeVisible) {
        if(width() > 15*MIN_WIDGET_DIM) mFillTypeCombo->show();
        else mFillTypeCombo->hide();
    }
}

void BoxSingleWidget::resizeEvent(QResizeEvent *) {
    updateCompositionBoxVisible();
    updatePathCompositionBoxVisible();
    updateFillTypeBoxVisible();
}
