#include "renderoutputwidget.h"
#include <QFileDialog>
#include <QLineEdit>

RenderOutputWidget::RenderOutputWidget(const int canvasWidth,
                                       const int canvasHeight,
                                       QWidget *parent) : QDialog(parent) {
    mCanvasWidth = canvasWidth;
    mCanvasHeight = canvasHeight;

    mPathLayout = new QHBoxLayout();
    mPathLabel = new QLabel(QDir::homePath(), this);
    mPathLayout->addWidget(mPathLabel);
    mSelectPathButton = new QPushButton("...", this);
    mPathLayout->addWidget(mSelectPathButton);
    mRenderButton = new QPushButton("Render", this);
    connect(mSelectPathButton, &QPushButton::pressed,
            this, &RenderOutputWidget::chooseDir);
    connect(mRenderButton, &QPushButton::pressed,
            this, &RenderOutputWidget::emitRender);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    // size

    mWidthLabel = new QLabel("Width:", this);
    mWidthSpinBox = new QSpinBox(this);
    mWidthSpinBox->setRange(1, 9999);
    mWidthSpinBox->setValue(mCanvasWidth);

    mHeightLabel = new QLabel("Height:", this);
    mHeightSpinBox = new QSpinBox(this);
    mHeightSpinBox->setRange(1, 9999);
    mHeightSpinBox->setValue(mCanvasHeight);

    mSizeLayout = new QHBoxLayout();
    mSizeLayout->addWidget(mWidthLabel);
    mSizeLayout->addWidget(mWidthSpinBox);
    mSizeLayout->addWidget(mHeightLabel);
    mSizeLayout->addWidget(mHeightSpinBox);

    mResoultionLabel = new QLabel("Resolution:", this);

    mResolutionComboBox = new QComboBox(this);
    mResolutionComboBox->addItem("100 %");
    mResolutionComboBox->addItem("75 %");
    mResolutionComboBox->addItem("50 %");
    mResolutionComboBox->addItem("25 %");
    mResolutionComboBox->setEditable(true);
    mResolutionComboBox->lineEdit()->setInputMask("D00 %");
    mResolutionComboBox->setProperty("forceHandleEvent", QVariant(true));
    mResolutionComboBox->setCurrentText("100 %");
    mResolutionComboBox->setInsertPolicy(QComboBox::NoInsert);
    mResolutionComboBox->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);

    connectSignals();

    mSizeLayout->addWidget(mResoultionLabel);
    mSizeLayout->addWidget(mResolutionComboBox);

    // size

    mMainLayout->addLayout(mSizeLayout);
    mMainLayout->addLayout(mPathLayout);
    mMainLayout->addWidget(mRenderButton);
}

qreal RenderOutputWidget::getCurrentResolution() {
    return mCurrentResolutionFrac;
}

void RenderOutputWidget::connectSignals() {
    connect(mHeightSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &RenderOutputWidget::updateSizeBoxesFromHeight);
    connect(mWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &RenderOutputWidget::updateSizeBoxesFromWidth);
    connect(mResolutionComboBox, &QComboBox::currentTextChanged,
            this, &RenderOutputWidget::updateSizeBoxesFromResolution);
}

void RenderOutputWidget::disconnectSignals() {
    disconnect(mHeightSpinBox, qOverload<int>(&QSpinBox::valueChanged),
               this, &RenderOutputWidget::updateSizeBoxesFromHeight);
    disconnect(mWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged),
               this, &RenderOutputWidget::updateSizeBoxesFromWidth);
    disconnect(mResolutionComboBox, &QComboBox::currentTextChanged,
               this, &RenderOutputWidget::updateSizeBoxesFromResolution);
}

void RenderOutputWidget::emitRender() {
    emit renderOutput(mPathLabel->text() + "/",
                      getCurrentResolution());
    delete this;
}

void RenderOutputWidget::chooseDir() {
    QString dir = QFileDialog::getExistingDirectory(
                this, tr("Open Directory"),
                mPathLabel->text(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks);
    if(dir.isNull() ) return;
    mPathLabel->setText(dir);
}

void RenderOutputWidget::updateSizeBoxesFromHeight() {
    disconnectSignals();
    mCurrentResolutionFrac = mHeightSpinBox->value()/mCanvasHeight;
    mResolutionComboBox->setCurrentText(
                QString::number(qRound(mCurrentResolutionFrac*100)) + " %");
    mWidthSpinBox->setValue(qRound(mCurrentResolutionFrac*mCanvasWidth));
    connectSignals();
}

void RenderOutputWidget::updateSizeBoxesFromWidth() {
    disconnectSignals();
    mCurrentResolutionFrac = mWidthSpinBox->value()/mCanvasWidth;
    mResolutionComboBox->setCurrentText(
                QString::number(qRound(mCurrentResolutionFrac*100)) + " %");
    mHeightSpinBox->setValue(qRound(mCurrentResolutionFrac*mCanvasHeight));
    connectSignals();
}

void RenderOutputWidget::updateSizeBoxesFromResolution() {
    disconnectSignals();
    mCurrentResolutionFrac =
            mResolutionComboBox->currentText().remove(" %").toDouble()*0.01;
    mWidthSpinBox->setValue(qRound(mCurrentResolutionFrac*mCanvasWidth));
    mHeightSpinBox->setValue(qRound(mCurrentResolutionFrac*mCanvasHeight));
    connectSignals();
}
