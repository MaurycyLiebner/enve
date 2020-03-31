#ifndef BOOKMARKEDCOLORS_H
#define BOOKMARKEDCOLORS_H
#include "../bookmarkedwidget.h"

class BookmarkedColors : public BookmarkedWidget {
    Q_OBJECT
public:
    BookmarkedColors(const bool vertical,
                     const int dimension,
                     QWidget* const parent);

    void setCurrentColor(const QColor& color);
    void addColor(const QColor& color);
    void removeColor(const QColor& color);
};

#endif // BOOKMARKEDCOLORS_H
