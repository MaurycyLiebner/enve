#include "key.h"

#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"

Key::Key(Animator *parentAnimator) :
    SmartPointerTarget() {
    mParentAnimator = parentAnimator;
    mRelFrame = 0;
}

Key::~Key() {

}

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
int Key::saveToSql(int parentAnimatorSqlId) {
//    QSqlQuery query;
//    if(!query.exec(
//        QString("INSERT INTO Key (value, frame, endenabled, "
//                "startenabled, ctrlsmode, endvalue, endframe, startvalue, "
//                "startframe, qrealanimatorid) "
//                "VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10)").
//                arg(mValue, 0, 'f').
//                arg(mRelFrame).
//                arg(boolToSql(mEndEnabled)).
//                arg(boolToSql(mStartEnabled)).
//                arg(mCtrlsMode).
//                arg(mEndValue, 0, 'f').
//                arg(mEndFrame).
//                arg(mStartValue, 0, 'f').
//                arg(mStartFrame).
//                arg(parentAnimatorSqlId) ) ) {
//        qDebug() << query.lastError() << endl << query.lastQuery();
//    }

//    return query.lastInsertId().toInt();
}

#include <QSqlRecord>
void Key::loadFromSql(int keyId) {
//    QSqlQuery query;

//    QString queryStr = "SELECT * FROM Key WHERE id = " +
//            QString::number(keyId);
//    if(query.exec(queryStr)) {
//        query.next();
//        int idValue = query.record().indexOf("value");
//        int idFrame = query.record().indexOf("frame");
//        int idEndEnabled = query.record().indexOf("endenabled");
//        int idStartEnabled = query.record().indexOf("startenabled");
//        int idCtrlsMode = query.record().indexOf("ctrlsmode");
//        int idEndValue = query.record().indexOf("endvalue");
//        int idEndFrame = query.record().indexOf("endframe");
//        int idStartValue = query.record().indexOf("startvalue");
//        int idStartFrame = query.record().indexOf("startframe");

//        mValue = query.value(idValue).toReal();
//        mRelFrame = query.value(idFrame).toInt();
//        mEndEnabled = query.value(idEndEnabled).toBool();
//        mStartEnabled = query.value(idStartEnabled).toBool();
//        mCtrlsMode = static_cast<CtrlsMode>(query.value(idCtrlsMode).toInt());
//        mEndValue = query.value(idEndValue).toReal();
//        mEndFrame = query.value(idEndFrame).toInt();
//        mStartValue = query.value(idStartValue).toReal();
//        mStartFrame = query.value(idStartFrame).toInt();
//    } else {
//        qDebug() << "Could not load Key with id " << keyId;
//    }
}

bool Key::isSelected() { return mIsSelected; }

void Key::copyToContainer(KeysClipboardContainer *container) {
    container->copyKeyToContainer(this);
}

void Key::removeFromAnimator() {
    if(mParentAnimator == NULL) return;
    mParentAnimator->anim_removeKey(this);
}

void Key::setParentKey(ComplexKey *parentKey) {
    mParentKey = parentKey;
}

ComplexKey *Key::getParentKey() {
    return mParentKey;
}

Key *Key::getNextKey() {
    return mParentAnimator->anim_getNextKey(this);
}

Key *Key::getPrevKey() {
    return mParentAnimator->anim_getPrevKey(this);
}

bool Key::isAncestorSelected() {
    if(mParentKey == NULL) return isSelected();
    if(isSelected()) return true;
    return mParentKey->isAncestorSelected();
}

bool Key::hasPrevKey() {
    if(mParentAnimator == NULL) return false;
    return mParentAnimator->anim_hasPrevKey(this);
}

bool Key::hasNextKey() {
    if(mParentAnimator == NULL) return false;
    return mParentAnimator->anim_hasNextKey(this);
}

void Key::incFrameAndUpdateParentAnimator(int inc) {
    if(mParentAnimator == NULL) return;
    if((mParentKey == NULL) ? false : mParentKey->isAncestorSelected() ) return;
    mParentAnimator->anim_moveKeyToFrame(this, mRelFrame + inc);
}

void Key::addToSelection(QList<Key *> *selectedKeys) {
    if(isSelected()) return;
    setSelected(true);
    selectedKeys->append(this);
    incNumberPointers();
}

void Key::removeFromSelection(QList<Key *> *selectedKeys) {
    if(isSelected()) {
        setSelected(false);
        if(selectedKeys->removeOne(this) ) {
            decNumberPointers();
        }
    }
}

Animator *Key::getParentAnimator() {
    return mParentAnimator;
}

void Key::startFrameTransform() {
    mSavedRelFrame = mRelFrame;
}

void Key::cancelFrameTransform() {
    mParentAnimator->anim_moveKeyToFrame(this, mSavedRelFrame);
}

void Key::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame,
        const qreal &scaleFactor) {
    int newFrame =
            qRound(mSavedRelFrame +
                  (mSavedRelFrame -
                   mParentAnimator->
                   prp_absFrameToRelFrame(relativeToFrame))*
                   scaleFactor);
    if(newFrame == mRelFrame) return;
    incFrameAndUpdateParentAnimator(newFrame - mRelFrame);
}

void Key::setSelected(bool bT) {
    mIsSelected = bT;
}
#include "undoredo.h"
void Key::finishFrameTransform()
{
    if(mParentAnimator == NULL) return;
    mParentAnimator->addUndoRedo(
                new ChangeKeyFrameUndoRedo(mSavedRelFrame,
                                           mRelFrame, this));
}

int Key::getAbsFrame() {
    return mParentAnimator->prp_relFrameToAbsFrame(mRelFrame);
}

int Key::getRelFrame() {
    return mRelFrame;
}

void Key::setRelFrame(int frame) {
    if(frame == mRelFrame) return;
    mRelFrame = frame;
    if(mParentAnimator == NULL) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
}

void Key::setAbsFrame(const int &frame) {
    setRelFrame(mParentAnimator->prp_absFrameToRelFrame(frame));
}
