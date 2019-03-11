#ifndef BRUSHSELECTIONICON_H
#define BRUSHSELECTIONICON_H
#include "brushselectionwidget.h"
#include <QWidget>
class BrushSelectionIcon : public QWidget {
    Q_OBJECT
public:
    explicit BrushSelectionIcon(QWidget *parent = nullptr);

signals:
    void currentBrushChanged(BrushWrapper *);
public slots:
    void brushSelected(BrushWrapper * const wrapper);
private slots:
    void openDialog();
private:
    BrushWidget* mIcon = nullptr;
    BrushSelectionWidget* mSelectionWidget = nullptr;
};

#endif // BRUSHSELECTIONICON_H
