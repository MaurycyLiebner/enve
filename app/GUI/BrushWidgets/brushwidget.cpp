#include "brushwidget.h"
#include <QPainter>
#include <QMouseEvent>

BrushWidget::BrushWidget(BrushContexedWrapper * const brushCWrapper,
                         QWidget * const parent) :
    QWidget(parent), mBrushCWrapper(brushCWrapper) {
    setFixedSize(64, 64);
    connect(brushCWrapper, &BrushContexedWrapper::selectionChanged,
            this, &BrushWidget::selectionChanged);
}

void BrushWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    const auto& brushData = mBrushCWrapper->getBrushData();
    p.drawImage(QRect(0, 0, width(), height()), brushData.fIcon);

    if(isSelected()) {
        if(mHovered) {
            p.setPen(QPen(Qt::red, 1, Qt::DashLine));
            p.drawRect(2, 2, width() - 5, height() - 5);
        }
        p.setPen(QPen(Qt::red, 2));
        p.drawRect(1, 1, width() - 2, height() - 2);
    } else if(mHovered) {
        p.setPen(QPen(Qt::red, 1, Qt::DashLine));
        p.drawRect(0, 0, width() - 1, height() - 1);
    }

    p.end();
}

void BrushWidget::mousePressEvent(QMouseEvent *e) {
    const auto pressedButton = e->button();
    if(pressedButton == Qt::RightButton) {

    } else if(pressedButton == Qt::LeftButton) {
        mBrushCWrapper->setSelected(true);
    } else return;
    update();
}
