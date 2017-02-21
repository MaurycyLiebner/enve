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

    mFillWidget = new QWidget(this);
    mMainLayout->addWidget(mFillWidget);
    mFillWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0)");

    mValueSlider = new QrealAnimatorValueSlider(NULL, this);
    mMainLayout->addWidget(mValueSlider, Qt::AlignRight);

    mColorButton = new BoxesListActionButton(this);
    mMainLayout->addWidget(mColorButton, Qt::AlignRight);
    connect(mColorButton, SIGNAL(pressed()),
            this, SLOT(openColorSettingsDialog()));

    mMainLayout->addSpacing(10);

    hide();
}

void BoxSingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    SingleWidget::setTargetAbstraction(abs);
    SingleWidgetTarget *target = abs->getTarget();
    const SWT_Type &type = target->SWT_getType();

    if(type == SWT_BoundingBox) {
        //BoundingBox *bb_target = (BoundingBox*)target;

        mRecordButton->hide();

        mContentButton->show();

        mVisibleButton->show();

        mColorButton->hide();

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(type == SWT_BoxesGroup) {
        //BoxesGroup *bg_target = (BoxesGroup*)target;

        //setName(bg_target->getName());

        mRecordButton->hide();

        mContentButton->show();

        mVisibleButton->show();

        mColorButton->hide();

        mValueSlider->setAnimator(NULL);
        mValueSlider->hide();
    } else if(type == SWT_QrealAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;

        mRecordButton->show();

        mContentButton->hide();

        mVisibleButton->hide();

        mColorButton->hide();

        mValueSlider->setAnimator(qa_target);
        mValueSlider->show();
    } else if(type == SWT_ComplexAnimator ||
              type == SWT_ColorAnimator) {
        //ComplexAnimator *ca_target = (ComplexAnimator*)target;

        mRecordButton->show();

        mContentButton->show();

        mVisibleButton->hide();

        if(type == SWT_ColorAnimator) {
            mColorButton->show();
        } else {
            mColorButton->hide();
        }

        mValueSlider->setAnimator(NULL);
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
void BoxSingleWidget::mousePressEvent(QMouseEvent *event)
{
    SingleWidgetTarget *target = mTarget->getTarget();
    const SWT_Type &type = target->SWT_getType();
    if(event->button() == Qt::RightButton) {
        QMenu menu(this);

        if(type == SWT_BoundingBox ||
           type == SWT_BoxesGroup) {
            menu.addAction("Rename");
        }
        QAction *selected_action = menu.exec(event->globalPos());
        if(selected_action != NULL)
        {
            if(selected_action->text() == "Rename") {
                rename();
            }
        } else {

        }
    } else {
        mDragStartPos = event->pos();
        if(type == SWT_BoundingBox ||
           type == SWT_BoxesGroup) {
            BoundingBox *bb_target = (BoundingBox*)target;
            bb_target->selectionChangeTriggered(event->modifiers() &
                                                Qt::ShiftModifier);
        }
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

    drag->setMimeData(mTarget->getTarget()->SWT_createMimeData());

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void BoxSingleWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->modifiers() & Qt::ShiftModifier) {
        mousePressEvent(e);
    } else {
        rename();
        MainWindow::getInstance()->callUpdateSchedulers();
    }
}

void BoxSingleWidget::rename() {
    SingleWidgetTarget *target = mTarget->getTarget();
    const SWT_Type &type = target->SWT_getType();
    if(type == SWT_BoundingBox ||
       type == SWT_BoxesGroup) {
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
    const SWT_Type &type = target->SWT_getType();
    if(type == SWT_BoundingBox ||
       type == SWT_BoxesGroup) {
        BoundingBox *bb_target = (BoundingBox*)target;
        bb_target->drawKeys(p, pixelsPerFrame,
                            containerTop,
                            minViewedFrame, maxViewedFrame);
    } else if(type == SWT_QrealAnimator ||
              type == SWT_ComplexAnimator ||
              type == SWT_ColorAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;
        qa_target->drawKeys(p, pixelsPerFrame,
                            containerTop,
                            minViewedFrame, maxViewedFrame);
    }
}

QrealKey *BoxSingleWidget::getKeyAtPos(const int &pressX,
                                       const qreal &pixelsPerFrame,
                                       const int &minViewedFrame) {
    if(isHidden()) return NULL;
    SingleWidgetTarget *target = mTarget->getTarget();
    const SWT_Type &type = target->SWT_getType();
    if(type == SWT_BoundingBox ||
       type == SWT_BoxesGroup) {
        BoundingBox *bb_target = (BoundingBox*)target;
        return bb_target->getAnimatorsCollection()->getKeyAtPos(
                    pressX,
                    minViewedFrame,
                    pixelsPerFrame);
    } else if(type == SWT_QrealAnimator ||
              type == SWT_ComplexAnimator ||
              type == SWT_ColorAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;
        return qa_target->getKeyAtPos(pressX,
                               minViewedFrame,
                               pixelsPerFrame);
    }
    return NULL;
}

void BoxSingleWidget::getKeysInRect(QRectF selectionRect,
                                    qreal pixelsPerFrame,
                                    const int &minViewedFrame,
                                    QList<QrealKey *> *listKeys) {
    if(isHidden()) return;
    SingleWidgetTarget *target = mTarget->getTarget();
    const SWT_Type &type = target->SWT_getType();
    if(type == SWT_BoundingBox ||
       type == SWT_BoxesGroup) {
        BoundingBox *bb_target = (BoundingBox*)target;
        bb_target->getAnimatorsCollection()->getKeysInRect(
                    selectionRect,
                    minViewedFrame,
                    pixelsPerFrame,
                    listKeys);
    } else if(type == SWT_QrealAnimator ||
              type == SWT_ComplexAnimator ||
              type == SWT_ColorAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;
        qa_target->getKeysInRect(selectionRect,
                                 minViewedFrame,
                                 pixelsPerFrame,
                                 listKeys);
    }
}


void BoxSingleWidget::paintEvent(QPaintEvent *) {
    if(mTarget == NULL) return;
    QPainter p(this);
    SingleWidgetTarget *target = mTarget->getTarget();
    const SWT_Type &type = target->SWT_getType();

    int nameX;
    QString name;
    if(type == SWT_BoundingBox ||
       type == SWT_BoxesGroup) {
        BoundingBox *bb_target = (BoundingBox*)target;

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

        nameX = mVisibleButton->x() + BOX_HEIGHT + 5;
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
    } */else if(type == SWT_QrealAnimator) {
        QrealAnimator *qa_target = (QrealAnimator*)target;
        name = qa_target->getName();
        if(qa_target->isRecording()) {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_RECORDING);
        } else {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_NOT_RECORDING);
        }

        nameX = mRecordButton->x() + 2*BOX_HEIGHT;
        p.setPen(Qt::white);
    } else if(type == SWT_ComplexAnimator ||
              type == SWT_ColorAnimator) {
        ComplexAnimator *ca_target = (ComplexAnimator*)target;
        name = ca_target->getName();

        if(ca_target->isRecording()) {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_RECORDING);
        } else {
            p.drawPixmap(mRecordButton->x(), 0,
                         *BoxSingleWidget::ANIMATOR_NOT_RECORDING);
            if(ca_target->isDescendantRecording()) {
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
        nameX = mContentButton->x() + BOX_HEIGHT;
        p.setPen(Qt::white);

        if(type == SWT_ColorAnimator) {
            ColorAnimator *col_target = (ColorAnimator*)ca_target;
            p.setBrush(col_target->getCurrentValue().qcol);
            p.drawRect(mColorButton->x(), 3,
                       BOX_HEIGHT, BOX_HEIGHT - 6);
            nameX = mColorButton->x() + BOX_HEIGHT;
        }
    }
    p.drawText(QRect(mFillWidget->x(), 0,
                     width() - mFillWidget->x() -
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
    ((Animator*)mTarget->getTarget())->switchRecording();
    MainWindow::getInstance()->callUpdateSchedulers();
    update();
}

void BoxSingleWidget::switchBoxVisibleAction() {
    ((BoundingBox*)mTarget->getTarget())->switchVisible();
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
