#ifndef BRUSHARRAYWIDGET_H
#define BRUSHARRAYWIDGET_H

#include <QMouseEvent>
#include <QPainter>
#include "arraywidget.h"
#include "brushwidget.h"
#include "smartPointers/sharedpointerdefs.h"
class BrushSelectionWidget;

class BrushArrayWidget : public ArrayWidget {
    Q_OBJECT
public:
    BrushArrayWidget(const Qt::Orientation& orientation,
                     QWidget *parent = nullptr);

    void insertBrush(const int& i, const stdsptr<BrushWrapper>& brush) {
        BrushWidget* newWidget = BrushWidget::createWidget(brush, this);
        if(mNoDuplicated) removeBrush(brush);
        insertWidget(i, newWidget);
    }

    bool hasBrush(BrushWrapper* brush) {
        auto tester = [&brush](QWidget* widget) {
            return static_cast<BrushWidget*>(widget)->getItem() == brush;
        };
        return hasWidget(tester);
    }

    bool removeBrush(const stdsptr<BrushWrapper>& brush) {
        auto tester = [&brush](QWidget* widget) {
            return static_cast<BrushWidget*>(widget)->getItem() == brush.get();
        };
        return removeWidget(tester);
    }

    bool readBinary(QIODevice *dst,
                    BrushSelectionWidget* brushes);
    bool writeBinary(QIODevice *dst);
public slots:
    void appendBrush(const stdsptr<BrushWrapper>& brush) {
        BrushWidget* newWidget = BrushWidget::createWidget(brush, this);
        if(mNoDuplicated) removeBrush(brush);
        appendWidget(newWidget);
    }

    void prependBrush(const stdsptr<BrushWrapper>& brush) {
        BrushWidget* newWidget = BrushWidget::createWidget(brush, this);
        if(mNoDuplicated) removeBrush(brush);
        if(newWidget == nullptr) return;
        prependWidget(newWidget);
    }
signals:
    void brushSelected(BrushWrapper*);
protected:
    void connectChildWidget(QWidget *widget) {
        connect(static_cast<BrushWidget*>(widget), &BrushWidget::selected,
                this, &BrushArrayWidget::emitBrushSelected);
    }
private slots:
    void emitBrushSelected(StdSelfRef* item) {
        emit brushSelected(GetAsPtr(item, BrushWrapper));
    }
private:
    bool mNoDuplicated = true;
    using ArrayWidget::appendWidget;
    using ArrayWidget::prependWidget;
    using ArrayWidget::insertWidget;
};
#endif // BRUSHARRAYWIDGET_H
