#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"
#include "skiaincludes.h"
#include <QSqlError>

enum PathEffectType : short {
    DISCRETE_PATH_EFFECT,
    DASH_PATH_EFFECT,
    DUPLICATE_PATH_EFFECT,
    SUM_PATH_EFFECT
};

class PathEffect : public ComplexAnimator {
    Q_OBJECT
public:
    PathEffect(const PathEffectType &type);

    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;
    virtual void filterPath(const SkPath &, SkPath *) = 0;
    virtual void filterPathForRelFrame(const int &,
                                       const SkPath &,
                                       SkPath *) = 0;
    virtual int saveToSql(QSqlQuery *query,
                          const int &boundingBoxSqlId,
                          const bool &outline);
    virtual void loadFromSql(const int &identifyingId) = 0;
protected:
    PathEffectType mPathEffectType;
};

class DisplacePathEffect : public PathEffect {
    Q_OBJECT
public:
    DisplacePathEffect();

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QrealAnimator *segLen,
                                QrealAnimator *maxDev);

    void filterPath(const SkPath &src, SkPath *dst);
    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);

    int saveToSql(QSqlQuery *query,
                  const int &boundingBoxSqlId,
                  const bool &outline) {
        int pathEffectId = PathEffect::saveToSql(query,
                                                 boundingBoxSqlId,
                                                 outline);
        int segLenId = mSegLength->saveToSql(query);
        int maxDevId = mMaxDev->saveToSql(query);
        int smoothnessId = mSmoothness->saveToSql(query);

        if(!query->exec(
                    QString("INSERT INTO displacepatheffect "
                            "(patheffectid, seglengthid, "
                            "maxdevid, smoothnessid) "
                            "VALUES (%1, %2, %3, %4)").
                    arg(pathEffectId).
                    arg(segLenId).
                    arg(maxDevId).
                    arg(smoothnessId) ) ) {
            qDebug() << query->lastError() << endl << query->lastQuery();
        }

        return query->lastInsertId().toInt();
    }

    void loadFromSql(const int &pathEffectId) {
        QSqlQuery query;

        QString queryStr = "SELECT * FROM displacepatheffect WHERE patheffectid = " +
                QString::number(pathEffectId);
        if(query.exec(queryStr)) {
            query.next();
            mSegLength->loadFromSql(query.value("seglengthid").toInt());
            mMaxDev->loadFromSql(query.value("maxdevid").toInt());
            mSmoothness->loadFromSql(query.value("smoothnessid").toInt());
        } else {
            qDebug() << "Could not load displacepatheffect with id " << pathEffectId;
        }
    }
private:
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mSmoothness =
            (new QrealAnimator())->ref<QrealAnimator>();
    uint32_t mSeedAssist = 0;
};

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
public:
    DuplicatePathEffect();

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QPointFAnimator *trans);

    void filterPath(const SkPath &src, SkPath *dst);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);

    int saveToSql(QSqlQuery *query,
                  const int &boundingBoxSqlId,
                  const bool &outline) {
        int pathEffectId = PathEffect::saveToSql(query,
                                                 boundingBoxSqlId,
                                                 outline);
        int translationId = mTranslation->saveToSql(query);

        if(!query->exec(
                    QString("INSERT INTO duplicatepatheffect "
                            "(patheffectid, translationid) "
                            "VALUES (%1, %2)").
                    arg(pathEffectId).
                    arg(translationId) ) ) {
            qDebug() << query->lastError() << endl << query->lastQuery();
        }

        return query->lastInsertId().toInt();
    }

    void loadFromSql(const int &pathEffectId) {
        QSqlQuery query;

        QString queryStr = "SELECT * FROM duplicatepatheffect WHERE patheffectid = " +
                QString::number(pathEffectId);
        if(query.exec(queryStr)) {
            query.next();
            mTranslation->loadFromSql(query.value("translationid").toInt());
        } else {
            qDebug() << "Could not load duplicatepatheffect with id " << pathEffectId;
        }
    }
private:
    QSharedPointer<QPointFAnimator> mTranslation =
            (new QPointFAnimator())->ref<QPointFAnimator>();
};

class PathBox;
#include "Properties/boxtargetproperty.h"
class SumPathEffect : public PathEffect {
    Q_OBJECT
public:
    SumPathEffect(PathBox *parentPath);

    Property *makeDuplicate() {}

    void makeDuplicate(Property *target) {}

    void filterPath(const SkPath &src, SkPath *dst) {}

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);

    int saveToSql(QSqlQuery *query,
                  const int &boundingBoxSqlId,
                  const bool &outline);
    void loadFromSql(const int &pathEffectId);
private:
    PathBox *mParentPathBox;
    QSharedPointer<BoxTargetProperty> mBoxTarget =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
};

#endif // PATHEFFECT_H
