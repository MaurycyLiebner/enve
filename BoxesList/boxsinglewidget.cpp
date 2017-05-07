#include "boxsinglewidget.h"
#include "OptimalScrollArea/singlewidgetabstraction.h"
#include "OptimalScrollArea/singlewidgettarget.h"
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "Colors/ColorWidgets/colorsettingswidget.h"

#include "Boxes/boxesgroup.h"
#include "qrealanimatorvalueslider.h"

QPixmap *BoxSingleWidget::VISIBLE_PIXMAP;
QPixmap *BoxSingleWidget::INVISIBLE_PIXMAP;
QPixmap *BoxSingleWidget::HIDE_CHILDREN;
QPixmap *BoxSingleWidget::SHOW_CHILDREN;
QPixmap *BoxSingleWidget::LOCKED_PIXMAP;
QPixmap *BoxSingleWidget::UNLOCKED_PIXMAP;
QPixmap *BoxSingleWidget::ANIMATOR_CHILDREN_VISIBLE;
QPixmap *BoxSingleWidget::ANIMATOR_CHILDREN_HIDDEN;
QPixmap *BoxSingleWidget::ANIMATOR_RECORDING;
QPixmap *BoxSingleWidget::ANIMATOR_NOT_RECORDING;
bool BoxSingleWidget::mStaticPixmapsLoaded = false;
#include "mainwindow.h"

BoxSingleWidget::BoxSingleWidget(ScrollWidgetVisiblePart *parent) :
    SingleWidget(parent) {
    mMainLayout = new QHBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    mMainLayout->setMargin(0);
    mMainLayout->setAlignment(Qt::AlignLeft);

    mRecordButton = new BoxesListActionButton(this);
    mMainLayout->addWidget(mRecordButton);
    connect(mRecordButton, SIGNAL(pressed()),
            this, SLOT(switchRecordingAction()));

    mContentButton = new BoxesListActionButton(this);
    mMainLayout->addWidget(mContentButton);
    connect(mContentButton, SIGNAL(pressed()),
            this, SLOT(switchContentVisibleAction()));

    mVisibleButton = new BoxesListActionButton(this);
    mMainLayout->addWidget(mVisibleButton);
    connect(mVisibleButton, SIGNAL(pressed()),
            this, SLOT(switchBoxVisibleAction()));

    mLockedButton = new BoxesListActionButton(this);
    mMainLayout->addWidget(mLockedButton);
    connect(mLockedButton, SIGNAL(pressed()),
            this, SLOT(switchBoxLockedAction()));

    mFillWidget = new QWidget(this);
    mMainLayout->addWidget(mFillWidget);
    mFillWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0)");

    mValueSlider = new QrealAnimatorValueSlider(NULL, this);
    mMainLayout->addWidget(mValueSlider, Qt::AlignRight);

    mColorButton = new BoxesListActionButton(this);
    mMainLayout->addWidget(mColorButton, Qt::AlignRight);
    connect(mColorButton, SIGNAL(pressed()),
            this, SLOT(openColorSettingsDialog()));

    mCompositionModeCombo = new QComboBox(this);
    mMainLayout->addWidget(mCompositionModeCombo);
    mCompositionModeCombo->addItems(QStringList() <<
                                    "Source Over" <<
                                    "Destination Over" <<
                                    "Clear" <<
                                    "Source" <<
                                    "Destination" <<
                                    "Source in" <<
                                    "Destination in" <<
                                    "Source Out" <<
                                    "Destination Out" <<
                                    "Source Atop" <<
                                    "Destination Atop" <<
                                    "Xor" <<
                                    "Plus" <<
                                    "Multiply" <<
                                    "Screen" <<
                                    "Overlay" <<
                                    "Darken" <<
                                    "Lighten" <<
                                    "Color Burn" <<
                                    "Hard Light" <<
                                    "Soft Light" <<
                                    "Difference" <<
                                    "Exclusion" <<
                                    "Source or Destination" <<
                                    "Source and Destination" <<
                                    "Source Xor Destination" <<
                                    "Not Source And Not Destination" <<
                                    "Not Source or Not Destination" <<
                                    "Not Source Xor Destination" <<
                                    "Not Source" <<
                                    "Not Source And Destination" <<
                                    "Source And Not Destination" <<
                                    "Not Source or Destination");
    connect(mCompositionModeCombo, SIGNAL(activated(int)),
            this, SLOT(setCompositionMode(int)));
    mCompositionModeCombo->setSizePolicy(QSizePolicy::Maximum,
                    mCompositionModeCombo->sizePolicy().horizontalPolicy());

    mBoxTargetWidget = new BoxTargetWidget(this);
    mMainLayout->addWidget(mBoxTargetWidget);

    mCheckBox = new BoolPropertyWidget(this);
    mMainLayout->addWidget(mCheckBox);

    mMainLayout->addSpacing(10);

    hide();
}

void BoxSingleWidget::setCompositionMode(const int &id) {
    SingleWidgetTarget *target = mTarget->getTarget();

    if(target->SWT_isBoundingBox()) {
        ((BoundingBox*)target)->setCompositionMode(
                    static_cast<QPainter::CompositionMode>(id));
    }
    MainWindow::getInstance()->callUpdateSchedulers();
}

void BoxSingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    SingleWidget::setTargetAbstraction(abs);
    SingleWidgetTarget *target = abs->getTarget();

    if(target->SWT_isBoxesGroup()) {
        //BoxesGroup *bg_target = (BoxesGroup*)target;

        //setName(bg_target->getName());

        mRecordButton->hide();

        mContentButton->show();

        mVisibleButton->show();

        mLockedButton->show();

        mColorButton->hide();

        mCompositionModeVisible = true;
        mCompositionModeCombo->setCurrentIndex(
                    ((BoundingBox*)target)->getCompositionMode());
        updateCompositionBoxVisible();

        mBoxTargetWidget->hide();
        mCheckBox->hide();

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(target->SWT_isBoundingBox()) {
        //BoundingBox *bb_target = (BoundingBox*)target;

        mRecordButton->hide();

        mContentButton->show();

        mVisibleButton->show();

        mLockedButton->show();

        mColorButton->hide();

        mCompositionModeVisible = true;
        mCompositionModeCombo->setCurrentIndex(
                    ((BoundingBox*)target)->getCompositionMode());
        updateCompositionBoxVisible();

        mBoxTargetWidget->hide();
        mCheckBox->hide();

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(target->SWT_isQrealAnimator()) {
        QrealAnimator *qa_target = (QrealAnimator*)target;

        mRecordButton->show();

        mContentButton->hide();

        mVisibleButton->hide();

        mLockedButton->hide();

        mColorButton->hide();

        mCompositionModeCombo->hide();
        mCompositionModeVisible = false;

        mBoxTargetWidget->hide();
        mCheckBox->hide();

        mValueSlider->setAnimator(qa_target);
        mValueSlider->show();
    } else if(target->SWT_isComplexAnimator()) {
        //ComplexAnimator *ca_target = (ComplexAnimator*)target;

        mRecordButton->show();

        mContentButton->show();

        mVisibleButton->hide();

        mLockedButton->hide();

        if(target->SWT_isColorAnimator()) {
            mColorButton->show();
        } else {
            mColorButton->hide();
        }

        mCompositionModeCombo->hide();
        mCompositionModeVisible = false;

        mBoxTargetWidget->hide();
        mCheckBox->hide();

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(target->SWT_isBoxTargetProperty()) {
        mRecordButton->hide();

        mContentButton->hide();

        mVisibleButton->hide();

        mLockedButton->hide();

        mColorButton->hide();

        mCompositionModeCombo->hide();
        mCompositionModeVisible = false;

        mBoxTargetWidget->show();
        mBoxTargetWidget->setTargetProperty((BoxTargetProperty*)target);
        mCheckBox->hide();

        mValueSlider->hide();
    } else if(target->SWT_isBoolAnimator()) {
        mRecordButton->hide();

        mContentButton->hide();

        mVisibleButton->hide();

        mLockedButton->hide();

        mColorButton->hide();

        mCompositionModeCombo->hide();
        mCompositionModeVisible = false;

        mBoxTargetWidget->hide();

        mCheckBox->show();
        mCheckBox->setTarget((BoolProperty*)target);

        mValueSlider->hide();
    }
}

void BoxSingleWidget::loadStaticPixmaps() {
    if(mStaticPixmapsLoaded) return;
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
    mStaticPixmapsLoaded = true;
}

#include <QInputDialog>
#include <QMenu>
#include "mainwindow.h"
#include "clipboardcontainer.h"
#include "durationrectangle.h"
void BoxSingleWidget::mousePressEvent(QMouseEvent *event) {
    SingleWidgetTarget *target = mTarget->getTarget();
    if(event->button() == Qt::RightButton) {
        QMenu menu(this);

        if(target->SWT_isBoundingBox()) {
            menu.addAction("Rename");
            QAction *durRectAct = menu.addAction("Visibility Range");
            durRectAct->setCheckable(true);
            durRectAct->setChecked(
                        ((BoundingBox*)target)->hasDurationRectangle());
        } else if(target->SWT_isAnimator()) {
            AnimatorClipboardContainer *clipboard =
                    (AnimatorClipboardContainer*)
                    MainWindow::getInstance()->getClipboardContainer(
                                                CCT_ANIMATOR);
            menu.addAction("Copy");
            if(clipboard != NULL) {
                menu.addAction("Paste");
            }
        }
        QAction *selected_action = menu.exec(event->globalPos());
        if(selected_action != NULL)
        {
            if(selected_action->text() == "Rename") {
                rename();
            } else if(selected_action->text() == "Visibility Range") {
                BoundingBox *boxTarget = (BoundingBox*)target;
                if(boxTarget->hasDurationRectangle()) {
                    boxTarget->setDurationRectangle(NULL);
                } else {
                    boxTarget->createDurationRectangle();
                }
            } else if(selected_action->text() == "Copy") {
                AnimatorClipboardContainer *container =
                        new AnimatorClipboardContainer();
                container->setAnimator((QrealAnimator*)target);
                MainWindow::getInstance()->replaceClipboard(container);
            } else if(selected_action->text() == "Paste") {
                AnimatorClipboardContainer *clipboard =
                        (AnimatorClipboardContainer*)
                        MainWindow::getInstance()->getClipboardContainer(
                                                    CCT_ANIMATOR);
                clipboard->paste((QrealAnimator*)target);
            }
        } else {

        }
    } else {
        mDragStartPos = event->pos();
//        if(type == SWT_BoundingBox ||
//           type == SWT_BoxesGroup) {
//            BoundingBox *bb_target = (BoundingBox*)target;
//            bb_target->selectionChangeTriggered(event->modifiers() &
//                                                Qt::ShiftModifier);
//        }
    }
    MainWindow::getInstance()->callUpdateSchedulers();
}

#include <QApplication>
#include <QDrag>
void BoxSingleWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - mDragStartPos).manhattanLength()
         < QApplication::startDragDistance()) {
        return;
    }
    QDrag *drag = new QDrag(this);

    QMimeData *mimeData = mTarget->getTarget()->SWT_createMimeData();
    if(mimeData == NULL) return;
    drag->setMimeData(mimeData);

    drag->installEventFilter(MainWindow::getInstance());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void BoxSingleWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(pointToLen(event->pos() - mDragStartPos) > 10.) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isBoundingBox()) {
        BoundingBox *bb_target = (BoundingBox*)target;
        bb_target->selectionChangeTriggered(event->modifiers() &
                                            Qt::ShiftModifier);
        MainWindow::getInstance()->callUpdateSchedulers();
    } else if(target->SWT_isQrealAnimator()) {
        QrealAnimator *qa_target = (QrealAnimator*)target;
        KeysView *keysView =
                ((BoxScrollWidgetVisiblePart*)mParent)->getKeysView();
        if(keysView != NULL) {
            if(qa_target->isCurrentAnimator(mParent)) {
                keysView->graphRemoveViewedAnimator(qa_target);
            } else {
                keysView->graphAddViewedAnimator(qa_target);
            }
            MainWindow::getInstance()->callUpdateSchedulers();
        }
    }
}

void BoxSingleWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->modifiers() & Qt::ShiftModifier) {
        //mousePressEvent(e);
    } else {
        rename();
        MainWindow::getInstance()->callUpdateSchedulers();
    }
}

void BoxSingleWidget::rename() {
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isBoundingBox()) {
        BoundingBox *bb_target = (BoundingBox*)target;
        bool ok;
        QString text = QInputDialog::getText(this, tr("New name dialog"),
                                             tr("Name:"), QLineEdit::Normal,
                                             bb_target->getName(), &ok);
        if(ok) {
            bb_target->setName(text);

            bb_target->
                    SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty();
        }
    }
}

void BoxSingleWidget::drawKeys(QPainter *p, qreal pixelsPerFrame,
                             int containerTop,
                             int minViewedFrame, int maxViewedFrame) {
    if(isHidden()) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        Animator *anim_target = (Animator*)target;
        anim_target->prp_drawKeys(p, pixelsPerFrame,
                            containerTop,
                            minViewedFrame, maxViewedFrame);
    }
}

Key *BoxSingleWidget::getKeyAtPos(const int &pressX,
                                  const qreal &pixelsPerFrame,
                                  const int &minViewedFrame) {
    if(isHidden()) return NULL;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        Animator *anim_target = (Animator*)target;
        return anim_target->prp_getKeyAtPos(pressX,
                               minViewedFrame,
                               pixelsPerFrame);
    }
    return NULL;
}

DurationRectangleMovable *BoxSingleWidget::getRectangleMovableAtPos(
                            const int &pressX,
                            const qreal &pixelsPerFrame,
                            const int &minViewedFrame) {
    if(isHidden()) return NULL;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        Animator *anim_target = (Animator*)target;
        return anim_target->anim_getRectangleMovableAtPos(
                                    pressX,
                                    minViewedFrame,
                                    pixelsPerFrame);
    }
    return NULL;
}

void BoxSingleWidget::getKeysInRect(const QRectF &selectionRect,
                                    const qreal &pixelsPerFrame,
                                    QList<Key *> *listKeys) {
    if(isHidden()) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    if(target->SWT_isAnimator()) {
        Animator *anim_target = (Animator*)target;

        anim_target->prp_getKeysInRect(selectionRect,
                                 pixelsPerFrame,
                                 listKeys);
    }
}

#include "keysview.h"
void BoxSingleWidget::paintEvent(QPaintEvent *) {
    if(mTarget == NULL) return;
    QPainter p(this);
    SingleWidgetTarget *target = mTarget->getTarget();

    int nameX = mFillWidget->x();
    QString name;
    if(target->SWT_isBoundingBox()) {
        BoundingBox *bb_target = (BoundingBox*)target;

        nameX += 5;
        name = bb_target->getName();

        p.fillRect(rect(), QColor(0, 0, 0, 50));

        if(mTarget->contentVisible()) {
            p.drawPixmap(mContentButton->x(), 0,
                         *BoxSingleWidget::HIDE_CHILDREN);
        } else {
            p.drawPixmap(mContentButton->x(), 0,
                         *BoxSingleWidget::SHOW_CHILDREN);
        }

        if(bb_target->isVisible()) {
            p.drawPixmap(mVisibleButton->x(), 0,
                         *BoxSingleWidget::VISIBLE_PIXMAP);
        } else {
            p.drawPixmap(mVisibleButton->x(), 0,
                         *BoxSingleWidget::INVISIBLE_PIXMAP);
        }

        if(bb_target->isLocked()) {
            p.drawPixmap(mLockedButton->x(), 0,
                         *BoxSingleWidget::LOCKED_PIXMAP);
        } else {
            p.drawPixmap(mLockedButton->x(), 0,
                         *BoxSingleWidget::UNLOCKED_PIXMAP);
        }

        if(bb_target->isSelected()) {
            p.fillRect(QRect(mFillWidget->pos(), mFillWidget->size()),
                       QColor(180, 180, 180));
            p.setPen(Qt::black);
        } else {
            p.setPen(Qt::white);
        }
//        QFont font = p.font();
//        font.setBold(true);
//        p.setFont(font);
    } /*else if(type == SWT_BoxesGroup) {
    } */else if(target->SWT_isQrealAnimator()) {
        QrealAnimator *qa_target = (QrealAnimator*)target;
        if(qa_target->isCurrentAnimator(mParent)) {
            p.fillRect(nameX + BOX_HEIGHT/4, BOX_HEIGHT/4,
                       BOX_HEIGHT/2, BOX_HEIGHT/2,
                       qa_target->getAnimatorColor(mParent));
        }
        name = qa_target->prp_getName();
        nameX += 20;
        if(qa_target->prp_isRecording()) {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_RECORDING);
        } else {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_NOT_RECORDING);
        }

        p.setPen(Qt::white);
    } else if(target->SWT_isComplexAnimator()) {
        ComplexAnimator *ca_target = (ComplexAnimator*)target;
        name = ca_target->prp_getName();

        if(ca_target->prp_isRecording()) {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_RECORDING);
        } else {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_NOT_RECORDING);
            if(ca_target->prp_isDescendantRecording()) {
                p.save();
                p.setRenderHint(QPainter::Antialiasing);
                p.setBrush(Qt::red);
                p.setPen(Qt::NoPen);
                p.drawEllipse(QPointF(10,
                                      10),
                               2.5, 2.5);
                p.restore();
            }
        }

        if(mTarget->contentVisible()) {
            p.drawPixmap(mContentButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_CHILDREN_VISIBLE);
        } else {
            p.drawPixmap(mContentButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_CHILDREN_HIDDEN);
        }
        p.setPen(Qt::white);

        if(target->SWT_isColorAnimator()) {
            ColorAnimator *col_target = (ColorAnimator*)ca_target;
            p.setBrush(col_target->getCurrentColor().qcol);
            p.drawRect(mColorButton->x(), 3,
                       BOX_HEIGHT, BOX_HEIGHT - 6);
        }
    } else if(target->SWT_isBoxTargetProperty()) {
        nameX += 40;
        name = ((BoxTargetProperty*)target)->prp_getName();
    } else if(target->SWT_isBoolProperty()) {
        nameX += 40;
        name = ((BoolProperty*)target)->prp_getName();
    }
    p.drawText(QRect(nameX, 0,
                     width() - nameX -
                     BOX_HEIGHT,
                     BOX_HEIGHT),
               name, QTextOption(Qt::AlignVCenter));

    p.end();
}

void BoxSingleWidget::switchContentVisibleAction() {
    mTarget->switchContentVisible();
    MainWindow::getInstance()->callUpdateSchedulers();
    //mParent->callUpdaters();
}

void BoxSingleWidget::switchRecordingAction() {
    ((Animator*)mTarget->getTarget())->prp_switchRecording();
    MainWindow::getInstance()->callUpdateSchedulers();
    update();
}

void BoxSingleWidget::switchBoxVisibleAction() {
    ((BoundingBox*)mTarget->getTarget())->switchVisible();
    MainWindow::getInstance()->callUpdateSchedulers();
    update();
}

void BoxSingleWidget::switchBoxLockedAction() {
    ((BoundingBox*)mTarget->getTarget())->switchLocked();
    MainWindow::getInstance()->callUpdateSchedulers();
    update();
}

void BoxSingleWidget::openColorSettingsDialog() {
    QDialog *dialog = new QDialog(this);
    dialog->setLayout(new QVBoxLayout(dialog));
    ColorSettingsWidget *colorSettingsWidget =
            new ColorSettingsWidget(dialog);
    colorSettingsWidget->setColorAnimatorTarget(
                (ColorAnimator*)mTarget->getTarget());
    dialog->layout()->addWidget(colorSettingsWidget);
    connect(MainWindow::getInstance(), SIGNAL(updateAll()),
            dialog, SLOT(update()));

    dialog->show();
}

void BoxSingleWidget::updateCompositionBoxVisible() {
    if(mCompositionModeVisible) {
        if(width() > 500) {
            mCompositionModeCombo->show();
        } else {
            mCompositionModeCombo->hide();
        }
    }
}

void BoxSingleWidget::resizeEvent(QResizeEvent *) {
    updateCompositionBoxVisible();
}
