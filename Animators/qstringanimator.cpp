#include "qstringanimator.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "undoredo.h"

QStringAnimator::QStringAnimator() : Animator() {

}

int QStringAnimator::saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    if(!query->exec(
        QString("INSERT INTO qstringanimator (currenttext) "
                "VALUES ('%1')").
                arg(mCurrentText) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    int thisSqlId = query->lastInsertId().toInt();

    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QStringKey*)key.get())->saveToSql(thisSqlId);
    }

    return thisSqlId;
}

void QStringAnimator::makeDuplicate(QStringAnimator *anim) {
    anim->prp_setName(prp_mName);
    anim->prp_setRecording(false);
    anim->setCurrentTextValue(mCurrentText, false);
    if(anim_mIsRecording) {
        anim->anim_setRecordingWithoutChangingKeys(anim_mIsRecording);
    }
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        QStringKey *duplicate =
                ((QStringKey*)key.get())->makeDuplicate(anim);
        anim->anim_appendKey(duplicate);
    }
}

void QStringAnimator::loadFromSql(const int &qstringAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qstringanimator WHERE id = " +
            QString::number(qstringAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int idQstringAnimatorId = query.record().indexOf("id");
        int currentText = query.record().indexOf("currenttext");

        loadKeysFromSql(query.value(idQstringAnimatorId).toInt() );

        if(anim_mKeys.isEmpty()) {
            setCurrentTextValue(query.value(currentText).toString());
        } else {
            anim_setRecordingWithoutChangingKeys(true, false);
            mCurrentText = getTextValueAtRelFrame(anim_mCurrentRelFrame);
        }
    } else {
        qDebug() << "Could not load qstringanimator with id " << qstringAnimatorId;
    }
}

void QStringAnimator::loadKeysFromSql(const int &qstringAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qstringkey WHERE qstringanimatorid = " +
            QString::number(qstringAnimatorId);
    if(query.exec(queryStr)) {
        int idId = query.record().indexOf("id");
        while(query.next() ) {
            int idT = query.value(idId).toInt();
            anim_appendKey(QStringKey::qStringKeyFromSql(idT, this));
        }
    } else {
        qDebug() << "Could not load qpointfanimator with id " << qstringAnimatorId;
    }
}

void QStringAnimator::prp_setAbsFrame(const int &frame) {
    Animator::prp_setAbsFrame(frame);
    if(prp_hasKeys()) {
        mCurrentText = getTextValueAtRelFrame(anim_mCurrentRelFrame);
    }
}

void QStringAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) prp_setRecording(true);

    if(anim_mKeyOnCurrentFrame == NULL) {
        anim_mKeyOnCurrentFrame = new QStringKey(mCurrentText,
                                                 anim_mCurrentRelFrame,
                                                 this);
        anim_appendKey(anim_mKeyOnCurrentFrame);
    } else {
        ((QStringKey*)anim_mKeyOnCurrentFrame)->setText(mCurrentText);
    }
}

void QStringAnimator::setCurrentTextValue(const QString &text,
                                          const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new ChangeTextUndoRedo(this,
                                           mCurrentText,
                                           text));
    }
    mCurrentText = text;
    if(prp_isRecording() && saveUndoRedo) {
        anim_saveCurrentValueAsKey();
    }
}

QString QStringAnimator::getCurrentTextValue() {
    return mCurrentText;
}

QString QStringAnimator::getTextValueAtRelFrame(const int &relFrame) {
    if(anim_mKeys.isEmpty()) {
        return mCurrentText;
    }
    QStringKey *key = (QStringKey *)anim_getPrevKey(relFrame);
    if(key == NULL) {
        key = (QStringKey *)anim_getNextKey(relFrame);
    }
    return key->getText();
}

void QStringAnimator::prp_getFirstAndLastIdenticalRelFrame(
                            int *firstIdentical,
                            int *lastIdentical,
                            const int &relFrame) {
    if(anim_mKeys.isEmpty()) {
        *firstIdentical = INT_MIN;
        *lastIdentical = INT_MAX;
    } else {
        int prevId;
        int nextId;
        anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId,
                                                relFrame);

        Key *prevKey = anim_mKeys.at(prevId).get();
        Key *nextKey = anim_mKeys.at(nextId).get();
        Key *prevPrevKey = nextKey;
        Key *prevNextKey = prevKey;

        int fId = relFrame;
        int lId = relFrame;

        while(true) {
            fId = prevKey->getRelFrame();
            prevPrevKey = prevKey;
            prevKey = prevKey->getPrevKey();
            if(prevKey == NULL) {
                fId = INT_MIN;
                break;
            }
            if(prevKey->differsFromKey(prevPrevKey)) break;
        }

        while(true) {
            lId = nextKey->getRelFrame();
            if(nextKey->differsFromKey(prevNextKey)) break;
            prevNextKey = nextKey;
            nextKey = nextKey->getNextKey();
            if(nextKey == NULL) {
                lId = INT_MAX;
                break;
            }
        }
        *firstIdentical = fId;
        *lastIdentical = lId;
    }
}

QStringKey::QStringKey(const QString &stringT,
                       const int &relFrame,
                       QStringAnimator *parentAnimator) :
    Key(parentAnimator) {
    mRelFrame = relFrame;
    mText = stringT;
}

int QStringKey::saveToSql(const int &parentAnimatorSqlId) {
    QSqlQuery query;
    if(!query.exec(
        QString("INSERT INTO qstringkey (string, frame, qstringanimatorid) "
                "VALUES ('%1', %2, %3)").
                arg(mText).
                arg(mRelFrame).
                arg(parentAnimatorSqlId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return query.lastInsertId().toInt();
}

QStringKey *QStringKey::qStringKeyFromSql(const int &keyId,
                                          QStringAnimator *animator) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qstringkey WHERE id = " +
            QString::number(keyId);
    if(query.exec(queryStr)) {
        query.next();
        int idText = query.record().indexOf("string");
        int idFrame = query.record().indexOf("frame");

        QString text = query.value(idText).toString();
        int relFrame = query.value(idFrame).toInt();
        return new QStringKey(text, relFrame, animator);
    } else {
        qDebug() << "Could not load qstringkey with id " << keyId;
    }
    return NULL;
}

QStringKey *QStringKey::makeDuplicate(QStringAnimator *anim) {
    return new QStringKey(mText, mRelFrame, anim);
}

bool QStringKey::differsFromKey(Key *key) {
    return ((QStringKey*)key)->getText() != mText;
}
