#include "renderoutputwidget.h"
#include <QFileDialog>

RenderOutputWidget::RenderOutputWidget(QWidget *parent) : QDialog(parent)
{
    mPathLayout = new QHBoxLayout();
    mPathLabel = new QLabel("/home/ailuropoda/tmp", this);
    mPathLayout->addWidget(mPathLabel);
    mSelectPathButton = new QPushButton("...", this);
    mPathLayout->addWidget(mSelectPathButton);
    mRenderButton = new QPushButton("Render", this);
    connect(mSelectPathButton, SIGNAL(pressed()),
            this, SLOT(chooseDir()));
    connect(mRenderButton, SIGNAL(pressed()),
            this, SLOT(emitRender()) );
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mMainLayout->addLayout(mPathLayout);
    mMainLayout->addWidget(mRenderButton);
}

void RenderOutputWidget::emitRender() {
    emit render(mPathLabel->text() + "/");
    delete this;
}

void RenderOutputWidget::chooseDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                mPathLabel->text(),
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    if(dir.isNull() ) return;
    mPathLabel->setText(dir);
}
