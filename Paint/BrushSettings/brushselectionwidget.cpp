#include "brushselectionwidget.h"
#include <QPainter>
#include <QResizeEvent>

#include "Paint/PaintLib/brush.h"

BrushSelectionWidget::BrushSelectionWidget(QWidget *parent) :
    QWidget(parent) {
    mDefaultBrush = new Brush();
    mCurrentBrush = mDefaultBrush;
    setFixedHeight(mDimension + mMargin);
}

const Brush *BrushSelectionWidget::getCurrentBrush() const {
    return mCurrentBrush;
}

void BrushSelectionWidget::duplicateCurrentBrush() {
    Brush *newBrush = new Brush;
    *newBrush = *mCurrentBrush;
    addBrush(newBrush);
    if(newBrush == mCurrentBrush) return;
    mCurrentBrush = newBrush;
    emit brushSelected(newBrush);
}

Brush *BrushSelectionWidget::getBrushAt(const QPoint &pos) {
    int brushId = pos.x()/(mDimension + mMargin) +
            (pos.y()/(mDimension + mMargin))*mNumberColumns;
    if(brushId < 0 || brushId >= mAllBrushes.count()) return nullptr;
    return mAllBrushes.at(brushId);
}

int BrushSelectionWidget::maxNumberColumns() const {
    return width()/(mDimension + mMargin);
}

int BrushSelectionWidget::maxNumberRows() const {
    if(mNumberColumns == 0) return 1;
    return mAllBrushes.count()/mNumberColumns + 1;
}

void BrushSelectionWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    QPen bluePen = QPen(Qt::blue, 1.);
    QPen defPen = p.pen();
    int brushId = 0;
    for(int i = 0; i < maxNumberRows(); i++) {
        for(int j = 0; j < mNumberColumns; j++) {
            if(brushId >= mAllBrushes.count()) {
                p.end();
                return;
            }
            Brush *brush = mAllBrushes.at(brushId);
            p.drawText(j*(mDimension + mMargin),
                       i*(mDimension + mMargin),
                       mDimension, mDimension,
                       Qt::AlignCenter | Qt::TextWrapAnywhere,
                       brush->getBrushName());
            if(brush == mCurrentBrush) {
                p.setPen(QPen(Qt::red, 2.));
                p.drawRect(j*(mDimension + mMargin) + 1,
                           i*(mDimension + mMargin) + 1,
                           mDimension - 1, mDimension - 1);
                p.setPen(defPen);
            }
            if(brush->isProjectOnly()) {
                p.setPen(bluePen);
                p.drawRect(j*(mDimension + mMargin),
                           i*(mDimension + mMargin),
                           mDimension, mDimension);
                p.setPen(defPen);
            }

            brushId++;
        }
    }

    p.end();
}
#include <QMenu>
#include <QInputDialog>
void BrushSelectionWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton) {
        Brush *pressedBrush = getBrushAt(event->pos());
        if(pressedBrush != nullptr) {
            mCurrentBrush = pressedBrush;
            emit brushSelected(pressedBrush);
            update();
        }
    }
    if(event->button() == Qt::RightButton) {
        QMenu menu;
        menu.addAction("New Brush...");
        QAction *action = menu.exec(event->globalPos());
        if(action != nullptr) {
            bool ok;
            QString text = QInputDialog::getText(this, "Brush Name",
                                                 "Brush name:", QLineEdit::Normal,
                                                 "", &ok);
            while(text.isEmpty() && ok) {
                text = QInputDialog::getText(this, "Brush Name",
                                             "Brush name:", QLineEdit::Normal,
                                             "", &ok);
            }
            if(ok) {
                duplicateCurrentBrush();
                mCurrentBrush->setBrushName(text);
            }
        }
    }
}

void BrushSelectionWidget::resizeEvent(QResizeEvent *) {
    mNumberColumns = maxNumberColumns();
}

void BrushSelectionWidget::addBrush(Brush *brush) {
    mAllBrushes.append(brush);
    if(mCurrentBrush == mDefaultBrush) {
        mCurrentBrush = brush;
        emit brushSelected(brush);
    }
    setFixedHeight(maxNumberRows()*(mDimension + mMargin));
    update();
}

void BrushSelectionWidget::removeCurrentBrush() {
    if(mCurrentBrush == mDefaultBrush) return;
    removeBrush(mCurrentBrush);
}

void BrushSelectionWidget::setCurrentBrush(const Brush *brush) {
    if(mCurrentBrush == brush) return;
    mCurrentBrush = const_cast<Brush*>(brush);
    update();
}

void BrushSelectionWidget::removeBrush(Brush *brush) {
    if(brush == nullptr) return;
    mAllBrushes.removeOne(brush);
    if(mCurrentBrush == brush) {
        if(mAllBrushes.isEmpty()) {
            mCurrentBrush = mDefaultBrush;
        } else {
            mCurrentBrush = mAllBrushes.last();
        }
    }
    emit brushReplaced(brush, mCurrentBrush);
    delete brush;
    setFixedHeight(maxNumberRows()*(mDimension + mMargin));
    update();
}

void BrushSelectionWidget::saveBrushesForProject(QIODevice *target) {
    int nBrushes = 0;
    foreach(Brush *brush, mAllBrushes) {
        if(brush->isProjectOnly()) {
            nBrushes++;
        }
    }
    target->write((const char*)&nBrushes, sizeof(int));
    foreach(Brush *brush, mAllBrushes) {
        if(brush->isProjectOnly()) {
            brush->writeBrush(target);
        }
    }
}

void BrushSelectionWidget::readBrushesForProject(QIODevice *target) {
    int nBrushes;
    target->read(reinterpret_cast<char*>(&nBrushes), sizeof(int));
    for(int i = 0; i < nBrushes; i++) {
        Brush *newBrush = new Brush();
        newBrush->readBrush(target);
        addBrush(newBrush);
    }
}
