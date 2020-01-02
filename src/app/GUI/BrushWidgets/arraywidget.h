// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ARRAYWIDGET_H
#define ARRAYWIDGET_H

#include <QWidget>

class QBoxLayout;
class QPushButton;

class ArrayWidget : public QWidget {
    Q_OBJECT
public:
    explicit ArrayWidget(const Qt::Orientation& orientation,
                         QWidget *parent = nullptr);

    QWidget* getWidgetAt(const int i) const {
        return mArrayWidgets.at(i);
    }

    int widgetsCount() const {
        return mArrayWidgets.count();
    }

    void setOrientation(const Qt::Orientation& orientation);

    void appendWidget(QWidget* wid);

    void prependWidget(QWidget* wid);

    void insertWidget(const int i, QWidget* wid);

    void removeRedundantWidgets();

    template <typename T>
    bool removeWidget(T tester) {
        for(QWidget* wid : mArrayWidgets) {
            if(tester(wid)) {
                mArrayWidgets.removeOne(wid);
                delete wid;
                return true;
            }
        }
        return false;
    }

    bool removeWidget(QWidget* widget);

    template <typename T>
    bool removeWidgets(T tester) {
        for(int i = 0; i < mArrayWidgets.count(); i++) {
            QWidget* wid = mArrayWidgets.at(i);
            if(tester(wid)) {
                mArrayWidgets.removeOne(wid);
                delete wid;
                i--;
            }
        }
        return false;
    }

    template <typename T>
    void sortWidgets(T tester) {
        std::sort(mArrayWidgets.begin(), mArrayWidgets.end(), tester);
        updateWidgetsVisibility();
    }

    void incFirstWidgetId(const int inc = 1);

    void decFirstWidgetId(const int dec = 1);

    void setFirstWidgetId(const int id);

    template <typename T>
    bool hasWidget(T tester) {
        for(QWidget* wid : mArrayWidgets) {
            if(tester(wid)) return true;
        }
        return false;
    }

    void setMaxCount(const int maxCount);
protected:
    void resizeEvent(QResizeEvent* event);

    virtual void connectChildWidget(QWidget* wid);
signals:

public slots:

private slots:
    void nextButtonPressed();

    void prevButtonPressed();
private:
    void updateButtonIcons();
    void updateWidgetsVisibility();
    void updateWidgetsHLayout();
    void updateWidgetsVLayout();

    int mMaxShownCount = 5;
    int mMaxCount = 10;
    int mFirstId = 0;
    Qt::Orientation mOrientation;
    QList<QWidget*> mArrayWidgets;

    QBoxLayout* mMainLayout = nullptr;

    QPushButton* mNextButton;
    QPushButton* mPrevButton;
};

#endif // ARRAYWIDGET_H
