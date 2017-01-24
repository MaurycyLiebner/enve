#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"

LinkBox::LinkBox(QString srcFile, BoxesGroup *parent) :
    BoxesGroup(parent) {
    mSrc = srcFile;
    reload();
    setType(TYPE_LINK);
    setName("Link " + srcFile);
}

void LinkBox::reload() {
    foreach(BoundingBox *box, mChildren) {
        box->decNumberPointers();
    }

    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(mSrc);
    db.open();

    loadChildrenFromSql(0, false);

    db.close();

    scheduleAwaitUpdate();
}

void LinkBox::changeSrc() {
    QString src = QFileDialog::getOpenFileName(mMainWindow,
                                               "Link File",
                                               "",
                                               "AniVect Files (*.av)");
    if(!src.isEmpty()) {
        setSrc(src);
    }
}

void LinkBox::setSrc(const QString &src) {
    mSrc = src;
    reload();
}
