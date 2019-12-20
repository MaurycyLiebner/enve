#ifndef BOOKMARKEDBRUSHES_H
#define BOOKMARKEDBRUSHES_H
#include "brushselectionwidget.h"
#include "brushwidget.h"

#include <QDialog>

class BookmarkedBrushes : public QDialog {
    Q_OBJECT
public:
    BookmarkedBrushes(BrushesContext* const brushesContext,
                      QWidget* const parent);
private:
    void updateWidth();

    QList<BrushWidget*> mWidgets;
    BrushesContext* const mBrushesContext;
};

#endif // BOOKMARKEDBRUSHES_H
