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

#include "edialogs.h"

#include "Private/esettings.h"

#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

class evIconProvider : public QFileIconProvider {
public:
    evIconProvider() {
        const auto iconDir = eSettings::sIconsDir();
        mEvIcon = QIcon(iconDir + "/enve.png");
    }

    QIcon icon(const QFileInfo & info) const {
        const QString name = info.fileName();
        if(name.endsWith(".ev") ||
           name.endsWith(".xev")) return mEvIcon;
        return QFileIconProvider::icon(info);
    }
private:
    QIcon mEvIcon;
};

class evFileFilterProxyModel : public QSortFilterProxyModel {
public:
    evFileFilterProxyModel(QObject* parent) :
        QSortFilterProxyModel(parent)
    { mProvider = new evIconProvider; }
    ~evFileFilterProxyModel()
    { delete mProvider; }
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
        Q_UNUSED(sourceRow)
        Q_UNUSED(sourceParent)
        const auto fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
        fileModel->setIconProvider(mProvider);
        return true;
    }
private:
    evIconProvider * mProvider;
};


QString eDialogs::openFile(const QString &title,
                           const QString &path,
                           const QString &filter) {

    QFileDialog dialog(nullptr, title, path);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setNameFilter(filter);
    dialog.setProxyModel(new evFileFilterProxyModel(&dialog));
    if(dialog.exec()) {
        const QStringList paths = dialog.selectedFiles();
        const QString openPath(paths.isEmpty() ? "" : paths.first());
        return openPath;
    }
    return "";
}

QStringList eDialogs::openFiles(const QString &title,
                                const QString &path,
                                const QString &filter) {
    QFileDialog dialog(nullptr, title, path);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(filter);
    dialog.setProxyModel(new evFileFilterProxyModel(&dialog));
    if(dialog.exec()) return dialog.selectedFiles();
    return QStringList();
}

QString eDialogs::openDir(const QString &title, const QString &path) {
    QFileDialog dialog(nullptr, title, path);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setProxyModel(new evFileFilterProxyModel(&dialog));
    if(dialog.exec()) {
        const QStringList paths = dialog.selectedFiles();
        const QString openPath(paths.isEmpty() ? "" : paths.first());
        return openPath;
    }
    return "";
}

QString eDialogs::saveFile(const QString &title,
                           const QString &path,
                           const QString &filter) {
    QFileDialog dialog(nullptr, title, path);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(filter);
    dialog.setProxyModel(new evFileFilterProxyModel(&dialog));
    if(dialog.exec()) {
        const QStringList paths = dialog.selectedFiles();
        const QString openPath(paths.isEmpty() ? "" : paths.first());
        return openPath;
    }
    return "";
}
