#include "paintsettings.h"
#include "undoredo.h"
#include "updatescheduler.h"
#include "Colors/helpers.h"
#include "Colors/ColorWidgets/gradientwidget.h"
#include "mainwindow.h"
#include "Colors/ColorWidgets/colorvaluerect.h"
#include "Animators/animatorupdater.h"
#include "skqtconversions.h"
#include "skiaincludes.h"

ColorSetting::ColorSetting() {
    mChangedValue = CVR_ALL;
}

ColorSetting::ColorSetting(const ColorMode &settingModeT,
                           const CVR_TYPE &changedValueT,
                           const qreal &val1T,
                           const qreal &val2T,
                           const qreal &val3T,
                           const qreal &alphaT,
                           const ColorSettingType &typeT,
                           ColorAnimator *excludeT) {
    mType = typeT;
    mSettingMode = settingModeT;
    mChangedValue = changedValueT;
    mVal1 = val1T;
    mVal2 = val2T;
    mVal3 = val3T;
    mAlpha = alphaT;
    mExclude = excludeT;
}

void ColorSetting::apply(ColorAnimator *target) const {
    if(target == mExclude) return;
    if(mType == CST_START) {
        startColorTransform(target);
    } else if(mType == CST_CHANGE) {
        changeColor(target);
    } else {
        finishColorTransform(target);
    }
}

const ColorSettingType &ColorSetting::getType() const { return mType; }

const ColorMode &ColorSetting::getSettingMode() const { return mSettingMode; }

const CVR_TYPE &ColorSetting::getChangedValue() const { return mChangedValue; }

const qreal &ColorSetting::getVal1() const { return mVal1; }

const qreal &ColorSetting::getVal2() const { return mVal2; }

const qreal &ColorSetting::getVal3() const { return mVal3; }

const qreal &ColorSetting::getAlpa() const { return mAlpha; }

void ColorSetting::finishColorTransform(ColorAnimator *target) const {
    changeColor(target);
    target->prp_finishTransform();
}

void ColorSetting::changeColor(ColorAnimator *target) const {
    const ColorMode &targetMode = target->getColorMode();
    if(targetMode == mSettingMode) {
        target->setCurrentVal1Value(mVal1);
        target->setCurrentVal2Value(mVal2);
        target->setCurrentVal3Value(mVal3);
    } else {
        qreal val1 = mVal1;
        qreal val2 = mVal2;
        qreal val3 = mVal3;
        if(targetMode == RGBMODE) {
            if(mSettingMode == HSVMODE) {
                qhsv_to_rgb(&val1, &val2, &val3);
            } else {
                qhsl_to_rgb(&val1, &val2, &val3);
            }
        } else if(targetMode == HSVMODE) {
            if(mSettingMode == HSLMODE) {
                qhsl_to_hsv(&val1, &val2, &val3);
            } else {
                qhsl_to_hsv(&val1, &val2, &val3);
            }
        } else if(targetMode == HSLMODE) {
            if(mSettingMode == HSVMODE) {
                qhsv_to_hsl(&val1, &val2, &val3);
            } else {
                qrgb_to_hsl(&val1, &val2, &val3);
            }
        }
        target->setCurrentVal1Value(val1);
        target->setCurrentVal2Value(val2);
        target->setCurrentVal3Value(val3);
    }
    target->setCurrentAlphaValue(mAlpha);
}

void ColorSetting::startColorTransform(ColorAnimator *target) const {
    const ColorMode &targetMode = target->getColorMode();
    if(targetMode == mSettingMode && mChangedValue != CVR_ALL) {
        if(mChangedValue == CVR_RED ||
                mChangedValue == CVR_HUE) {
            target->startVal1Transform();

            qreal targetVal2 = target->getVal2Animator()->qra_getCurrentValue();
            if(qAbs(targetVal2 - mVal2) > 0.001) {
                target->startVal2Transform();
            }
            qreal targetVal3 = target->getVal3Animator()->qra_getCurrentValue();
            if(qAbs(targetVal3 - mVal3) > 0.001) {
                target->startVal3Transform();
            }
        } else if(mChangedValue == CVR_GREEN ||
                  mChangedValue == CVR_HSVSATURATION ||
                  mChangedValue == CVR_HSLSATURATION) {
            target->startVal2Transform();

            qreal targetVal1 = target->getVal1Animator()->qra_getCurrentValue();
            if(qAbs(targetVal1 - mVal1) > 0.001) {
                target->startVal1Transform();
            }
            qreal targetVal3 = target->getVal3Animator()->qra_getCurrentValue();
            if(qAbs(targetVal3 - mVal3) > 0.001) {
                target->startVal3Transform();
            }
        } else if(mChangedValue == CVR_BLUE ||
                  mChangedValue == CVR_VALUE ||
                  mChangedValue == CVR_LIGHTNESS) {
            target->startVal3Transform();

            qreal targetVal1 = target->getVal1Animator()->qra_getCurrentValue();
            if(qAbs(targetVal1 - mVal1) > 0.001) {
                target->startVal1Transform();
            }
            qreal targetVal2 = target->getVal2Animator()->qra_getCurrentValue();
            if(qAbs(targetVal2 - mVal2) > 0.001) {
                target->startVal2Transform();
            }
        }
    } else {
        target->startVal1Transform();
        target->startVal2Transform();
        target->startVal3Transform();
    }
    qreal targetAlpha = target->getAlphaAnimator()->qra_getCurrentValue();
    if(qAbs(targetAlpha - mAlpha) > 0.001 ||
            mChangedValue == CVR_ALL ||
            mChangedValue == CVR_ALPHA) {
        target->startAlphaTransform();
    }
    changeColor(target);
}

Gradient::Gradient() : ComplexAnimator() {
    prp_setUpdater(new GradientUpdater(this));
    prp_blockUpdater();
    prp_setName("gradient");
    updateQGradientStops();
}

Gradient::~Gradient() {
}

Gradient::Gradient(const Color &color1, const Color &color2) :
    ComplexAnimator()
{
    prp_setUpdater(new GradientUpdater(this));
    prp_blockUpdater();
    prp_setName("gradient");
    addColorToList(color1, false);
    addColorToList(color2, false);
    updateQGradientStops();
}

Property *Gradient::makeDuplicate() {
    Gradient *newGradient = new Gradient();

    Q_FOREACH(ColorAnimator *color, mColors) {
        newGradient->addColorToList(
                    (ColorAnimator*)color->makeDuplicate(), false);
    }

    newGradient->updateQGradientStops();

    return newGradient;
}

Gradient::Gradient(const int &sqlIdT) :
    ComplexAnimator() {
    prp_setUpdater(new GradientUpdater(this));
    prp_blockUpdater();
    prp_setName("gradient");
    QSqlQuery query;
    mSqlId = sqlIdT;
    QString queryStr = QString("SELECT * FROM gradient WHERE id = %1").
            arg(mSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        queryStr = QString("SELECT colorid FROM gradientcolor WHERE "
                           "gradientid = %1 ORDER BY positioningradient ASC").
                arg(mSqlId);
        if(query.exec(queryStr) ) {
            int idColorId = query.record().indexOf("colorid");
            while(query.next()) {
                int colorId = query.value(idColorId).toInt();
                ColorAnimator *newAnimator = new ColorAnimator();
                newAnimator->prp_loadFromSql(colorId);
                addColorToList(newAnimator);
            }
        } else {
            qDebug() << "Could not load gradientcolors "
                        "for gradient with id " << mSqlId;
        }
    } else {
        qDebug() << "Could not load gradient with id " << mSqlId;
    }
    updateQGradientStops();
}

bool Gradient::isEmpty() const {
    return mColors.isEmpty();
}

void Gradient::prp_startTransform() {
    //savedColors = colors;
}

void Gradient::addColorToList(const Color &color,
                              const bool &saveUndoRedo) {
    ColorAnimator *newColorAnimator = new ColorAnimator();
    newColorAnimator->qra_setCurrentValue(color);
    addColorToList(newColorAnimator, saveUndoRedo);
}

void Gradient::addColorToList(ColorAnimator *newColorAnimator,
                              const bool &saveUndoRedo) {
    mColors << newColorAnimator;

    ca_addChildAnimator(newColorAnimator);

    if(saveUndoRedo) {
        addUndoRedo(new GradientColorAddedToListUndoRedo(this,
                                                         newColorAnimator));
    }
}

Color Gradient::getCurrentColorAt(int id) {
    return mColors.at(id)->getCurrentColor();
}

ColorAnimator *Gradient::getColorAnimatorAt(const int &id) {
    return mColors.at(id);
}

int Gradient::getColorCount() {
    return mColors.length();
}

QColor Gradient::getLastQGradientStopQColor() {
    return mQGradientStops.last().second;
}

QColor Gradient::getFirstQGradientStopQColor() {
    return mQGradientStops.first().second;
}

QGradientStops Gradient::getQGradientStops() {
    return mQGradientStops;
}

int Gradient::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    query->exec("INSERT INTO gradient DEFAULT VALUES");
    mSqlId = query->lastInsertId().toInt();
    int posInGradient = 0;
    Q_FOREACH(ColorAnimator *color, mColors) {
        int colorId = color->prp_saveToSql(query);
        query->exec(QString("INSERT INTO gradientcolor "
                            "(colorid, gradientid, positioningradient) "
                            "VALUES (%1, %2, %3)").
                    arg(colorId).
                    arg(mSqlId).
                    arg(posInGradient) );
        posInGradient++;
    }
    return mSqlId;
}

void Gradient::saveToSqlIfPathSelected(QSqlQuery *query) {
    Q_FOREACH(PathBox *path, mAffectedPaths) {
        BoundingBox *parent = (BoundingBox *) path;
        while(parent != NULL) {
            if(parent->isSelected()) {
                prp_saveToSql(query);
                return;
            }
            parent = parent->getParent();
        }
    }
}

void Gradient::swapColors(const int &id1, const int &id2,
                          const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new GradientSwapColorsUndoRedo(this, id1, id2));
    }
    ca_swapChildAnimators(mColors.at(id1), mColors.at(id2));
    mColors.swap(id1, id2);
    updateQGradientStops();
}

void Gradient::removeColor(const int &id) {
    removeColor(mColors.at(id));
}

void Gradient::removeColor(ColorAnimator *color,
                           const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new GradientColorRemovedFromListUndoRedo(
                        this, color));
    }
    ca_removeChildAnimator(color);
    emit resetGradientWidgetColorIdIfEquals(this, mColors.indexOf(color));
    mColors.removeOne(color);
    updateQGradientStops();
}

void Gradient::addColor(const Color &color) {
    addColorToList(color);
    updateQGradientStops();
}

void Gradient::replaceColor(const int &id, const Color &color) {
    mColors.at(id)->qra_setCurrentValue(color);
    updateQGradientStops();
}

bool Gradient::isInPaths(PathBox *path) {
    return mAffectedPaths.contains(path);
}

void Gradient::addPath(PathBox *path) {
    mAffectedPaths << path;
}

void Gradient::removePath(PathBox *path) {
    mAffectedPaths.removeOne(path);
}

bool Gradient::affectsPaths() {
    return !mAffectedPaths.isEmpty();
}

void Gradient::updatePaths() {
    Q_FOREACH (PathBox *path, mAffectedPaths) {
        //path->replaceCurrentFrameCache();
        path->updateDrawGradients();
        path->scheduleSoftUpdate();
    }
}

//void Gradient::finishTransform() {
//    ComplexAnimator::finishTransform();
//    //addUndoRedo(new ChangeGradientColorsUndoRedo(savedColors, colors, this));
//    //savedColors = colors;
//    callUpdateSchedulers();
//}

void Gradient::scheduleQGradientStopsUpdate() {
    if(mQGradientStopsUpdateNeeded) return;
    mQGradientStopsUpdateNeeded = true;
    addUpdateScheduler(new QGradientStopsUpdateScheduler(this) );
}

void Gradient::updateQGradientStopsIfNeeded() {
    if(mQGradientStopsUpdateNeeded) {
        mQGradientStopsUpdateNeeded = false;
        updateQGradientStops();
    }
}

void Gradient::startColorIdTransform(int id) {
    if(mColors.count() <= id || id < 0) return;
    mColors.at(id)->prp_startTransform();
}

void Gradient::updateQGradientStops() {
    mQGradientStops.clear();
    qreal inc = 1./(mColors.length() - 1.);
    qreal cPos = 0.;
    for(int i = 0; i < mColors.length(); i++) {
        mQGradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0., 1.),
                                    mColors.at(i)->getCurrentColor().qcol) );
        cPos += inc;
    }
    updatePaths();
}

void Gradient::updateQGradientStopsFinal() {
    mQGradientStops.clear();
    qreal inc = 1./(mColors.length() - 1.);
    qreal cPos = 0.;
    for(int i = 0; i < mColors.length(); i++) {
        mQGradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0., 1.),
                               mColors.at(i)->getCurrentColor().qcol) );
        cPos += inc;
    }
    Q_FOREACH(PathBox *path, mAffectedPaths) {
        path->updateDrawGradients();
        path->replaceCurrentFrameCache();
    }
}

int Gradient::getSqlId() {
    return mSqlId;
}

void Gradient::setSqlId(const int &id) {
    mSqlId = id;
}

PaintSettings::PaintSettings() : PaintSettings(Color(255, 255, 255),
                                               PaintType::FLATPAINT,
                                               NULL) {
}

PaintSettings::PaintSettings(const Color &colorT,
                             const PaintType &paintTypeT,
                             Gradient *gradientT) : ComplexAnimator() {
    prp_setName("fill");
    mColor->qra_setCurrentValue(colorT);
    mPaintType = paintTypeT;
    setGradientVar(gradientT);

    ca_addChildAnimator(mColor.data());
}

void PaintSettings::setPaintPathTarget(PathBox *path) {
    mColor->prp_setUpdater(new DisplayedFillStrokeSettingsUpdater(path));
    mColor->prp_blockUpdater();
}

void PaintSettings::makeDuplicate(Property *target) {
    PaintSettings *paintSettingsTarget = (PaintSettings*)target;
    paintSettingsTarget->duplicateColorAnimatorFrom(mColor.data());
    paintSettingsTarget->setGradient(mGradient.data());
    paintSettingsTarget->setPaintType(mPaintType);
}

void PaintSettings::duplicateColorAnimatorFrom(ColorAnimator *source) {
    source->makeDuplicate(mColor.data());
}

void PaintSettings::setTargetPathBox(PathBox *target) {
    mTarget = target;
}

void PaintSettings::setGradientVar(Gradient *grad) {
    if(!mGradient.isNull()) {
        ca_removeChildAnimator(mGradient.data());
        ca_removeChildAnimator((QrealAnimator*) mGradientPoints);
        mGradient->removePath(mTarget);
    }
    if(grad == NULL) {
        mGradient.reset();
    } else {
        mGradient = grad->ref<Gradient>();
    }
    if(!mGradient.isNull()) {
        ca_addChildAnimator(mGradient.data());
        ca_addChildAnimator((QrealAnimator*) mGradientPoints);
        mGradient->addPath(mTarget);
    }
}

void PaintSettings::prp_loadFromSql(const int &sqlId) {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM paintsettings WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idPaintType = query.record().indexOf("painttype");
        mPaintType = static_cast<PaintType>(query.value(idPaintType).toInt());
        int idColorId = query.record().indexOf("colorid");
        mColor->prp_loadFromSql(query.value(idColorId).toInt() );
        int idGradientId = query.record().indexOf("gradientid");
        if(!query.value(idGradientId).isNull()) {
            setGradientVar(getLoadedGradientBySqlId(
                        query.value(idGradientId).toInt()));
        }
    } else {
        qDebug() << "Could not load paintSettings with id " << sqlId;
    }
}

int PaintSettings::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    int colorId = mColor->prp_saveToSql(query);
    QString gradientId = (mGradient.isNull()) ? "NULL" :
                                               QString::number(
                                                   mGradient->getSqlId());
    query->exec(QString("INSERT INTO paintsettings "
                        "(painttype, colorid, gradientid) "
                        "VALUES (%1, %2, %3)").
                arg(mPaintType).
                arg(colorId).
                arg(gradientId) );
    return query->lastInsertId().toInt();
}

Color PaintSettings::getCurrentColor() const {
    return mColor->getCurrentColor();
}

PaintType PaintSettings::getPaintType() const {
    return mPaintType;
}

Gradient *PaintSettings::getGradient() const {
    return mGradient.data();
}

void PaintSettings::setGradient(Gradient *gradient,
                                const bool &saveUndoRedo) {
    if(gradient == mGradient.data()) return;

    if(saveUndoRedo) {
        addUndoRedo(new GradientChangeUndoRedo(mGradient.data(), gradient, this));
    }
    setGradientVar(gradient);

    if(mTarget->isSelected()) {
        mMainWindow->scheduleDisplayedFillStrokeSettingsUpdate();
    }
}

void PaintSettings::setCurrentColor(const Color &color) {
    mColor->qra_setCurrentValue(color);
}

void PaintSettings::setPaintType(const PaintType &paintType,
                                 const bool &saveUndoRedo) {
    if(paintType == mPaintType) return;

    if(mPaintType == FLATPAINT) {
        ca_removeChildAnimator(mColor.data());
    } else if(paintType == FLATPAINT) {
        ca_addChildAnimator(mColor.data());
    }
    if(saveUndoRedo) {
        addUndoRedo(new PaintTypeChangeUndoRedo(mPaintType, paintType,
                                                this));
        if(mPaintType == GRADIENTPAINT) {
            setGradient(NULL);
        }
    }

    mPaintType = paintType;
    mTarget->updateDrawGradients();
    if(mTarget->isSelected()) {
        mMainWindow->scheduleDisplayedFillStrokeSettingsUpdate();
    }
}

ColorAnimator *PaintSettings::getColorAnimator() {
    return mColor.data();
}

void PaintSettings::setGradientPoints(GradientPoints *gradientPoints) {
    mGradientPoints = gradientPoints;
}

StrokeSettings::StrokeSettings() : StrokeSettings(Color(0, 0, 0),
                                                  PaintType::FLATPAINT,
                                                  NULL) {
}

StrokeSettings::StrokeSettings(const Color &colorT,
                               const PaintType &paintTypeT,
                               Gradient *gradientT) :
    PaintSettings(colorT,
                  paintTypeT,
                  gradientT) {
    prp_setName("stroke");
    mLineWidth->qra_setCurrentValue(1.);
    mLineWidth->prp_setName("thickness");

    ca_addChildAnimator(mLineWidth.data());

    mLineWidth->qra_setValueRange(0., mLineWidth->getMaxPossibleValue());
}

void StrokeSettings::setLineWidthUpdaterTarget(PathBox *path) {
    mLineWidth->prp_setUpdater(new StrokeWidthUpdater(path));
    setPaintPathTarget(path);
}

void StrokeSettings::prp_loadFromSql(const int &strokeSqlId) {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM "
                               "strokesettings WHERE id = %1").
            arg(strokeSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idPaintSettingsId = query.record().indexOf("paintsettingsid");
        int idLineWidth = query.record().indexOf("linewidthanimatorid");
        int idCapStyle = query.record().indexOf("capstyle");
        int idJoinStyle = query.record().indexOf("joinstyle");
        int paintSettingsId = static_cast<PaintType>(
                    query.value(idPaintSettingsId).toInt());
        PaintSettings::prp_loadFromSql(paintSettingsId);
        mLineWidth->prp_loadFromSql(query.value(idLineWidth).toInt() );
        mCapStyle = static_cast<Qt::PenCapStyle>(query.value(idCapStyle).toInt());
        mJoinStyle = static_cast<Qt::PenJoinStyle>(query.value(idJoinStyle).toInt());
    } else {
        qDebug() << "Could not load strokesettings with id " << strokeSqlId;
    }
}

int StrokeSettings::prp_saveToSql(QSqlQuery *query,
                                  const int &parentId) {
    Q_UNUSED(parentId);
    int paintSettingsId = PaintSettings::prp_saveToSql(query);
    int lineWidthId = mLineWidth->prp_saveToSql(query);
    query->exec(QString("INSERT INTO strokesettings (linewidthanimatorid, "
                       "capstyle, joinstyle, paintsettingsid) "
                       "VALUES (%1, %2, %3, %4)").
                arg(lineWidthId).
                arg(mCapStyle).
                arg(mJoinStyle).
                arg(paintSettingsId) );
    return query->lastInsertId().toInt();
}

void StrokeSettings::setCurrentStrokeWidth(const qreal &newWidth) {
    mLineWidth->qra_setCurrentValue(newWidth);
}

void StrokeSettings::setCapStyle(const Qt::PenCapStyle &capStyle) {
    mCapStyle = capStyle;
}

void StrokeSettings::setJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    mJoinStyle = joinStyle;
}

void StrokeSettings::setStrokerSettings(QPainterPathStroker *stroker) {
    stroker->setWidth(mLineWidth->qra_getCurrentValue());
    stroker->setCapStyle(mCapStyle);
    stroker->setJoinStyle(mJoinStyle);
}

void StrokeSettings::setStrokerSettingsSk(SkStroke *stroker) {
    stroker->setWidth(mLineWidth->qra_getCurrentValue());
    stroker->setCap(QCapToSkCap(mCapStyle));
    stroker->setJoin(QJoinToSkJoin(mJoinStyle));
}

qreal StrokeSettings::getCurrentStrokeWidth() const {
    return mLineWidth->qra_getCurrentValue();
}

Qt::PenCapStyle StrokeSettings::getCapStyle() const {
    return mCapStyle;
}

Qt::PenJoinStyle StrokeSettings::getJoinStyle() const {
    return mJoinStyle;
}

QrealAnimator *StrokeSettings::getStrokeWidthAnimator() {
    return mLineWidth.data();
}

void StrokeSettings::setOutlineCompositionMode(
        const QPainter::CompositionMode &compositionMode) {
    mOutlineCompositionMode = compositionMode;
}

QPainter::CompositionMode StrokeSettings::getOutlineCompositionMode() {
    return mOutlineCompositionMode;
}

bool StrokeSettings::nonZeroLineWidth() {
    return !isZero(mLineWidth->qra_getCurrentValue());
}

void StrokeSettings::makeDuplicate(Property *target) {
    PaintSettings::makeDuplicate(target);
    StrokeSettings *strokeSettingsTarget = (StrokeSettings*)target;
    strokeSettingsTarget->duplicateLineWidthFrom(mLineWidth.data());
    strokeSettingsTarget->setCapStyle(mCapStyle);
    strokeSettingsTarget->setJoinStyle(mJoinStyle);
    strokeSettingsTarget->setOutlineCompositionMode(mOutlineCompositionMode);
}

void StrokeSettings::duplicateLineWidthFrom(QrealAnimator *source) {
    source->makeDuplicate(mLineWidth.data());
}

QrealAnimator *StrokeSettings::getLineWidthAnimator() {
    return mLineWidth.data();
}

PaintSetting::PaintSetting(const bool &targetFillSettings, const ColorSetting &colorSetting) {
    mTargetFillSettings = targetFillSettings;
    mColorSetting = colorSetting;
    mPaintType = FLATPAINT;
}

PaintSetting::PaintSetting(const bool &targetFillSettings) {
    mTargetFillSettings = targetFillSettings;
    mPaintType = NOPAINT;
}

PaintSetting::PaintSetting(const bool &targetFillSettings, Gradient *gradient) {
    mTargetFillSettings = targetFillSettings;
    mPaintType = GRADIENTPAINT;
    mGradient = gradient;
}

void PaintSetting::apply(PathBox *box) const {
    PaintSettings *fillSettings;
    if(mTargetFillSettings) {
        fillSettings = box->getFillSettings();
    } else {
        fillSettings = box->getStrokeSettings();
    }
    bool paintTypeChanged = fillSettings->getPaintType() != mPaintType;
    bool gradientChanged = false;

    if(mPaintType == FLATPAINT) {
        mColorSetting.apply(fillSettings->getColorAnimator());
    } else if(mPaintType == GRADIENTPAINT) {
        if(paintTypeChanged) {
            gradientChanged = true;
        } else {
            gradientChanged = fillSettings->getGradient() == mGradient;
        }
        fillSettings->setGradient(mGradient);
        if(paintTypeChanged) {
            if(mTargetFillSettings) {
                box->resetFillGradientPointsPos(true);
              } else {
                box->resetStrokeGradientPointsPos(true);
            }
        }
//        if(gradientChanged) {
//            box->updateDrawGradients();
//        }
    }
    if(paintTypeChanged) {
        if(paintTypeChanged) {
            fillSettings->setPaintType(mPaintType);
        }
    }
    if(gradientChanged) {
        if(mTargetFillSettings) {
            box->updateFillDrawGradient();
        } else {
            box->updateStrokeDrawGradient();
        }
    }
}

void PaintSetting::applyColorSetting(ColorAnimator *animator) const {
    mColorSetting.apply(animator);
}

UpdatePaintSettings::UpdatePaintSettings(const QColor &paintColorT, const PaintType &paintTypeT, const QLinearGradient &gradientT) {
    paintColor = paintColorT;
    paintType = paintTypeT;
    gradient = gradientT;
}

UpdatePaintSettings::UpdatePaintSettings() {}

UpdatePaintSettings::~UpdatePaintSettings() {}

void UpdatePaintSettings::applyPainterSettings(QPainter *p) {
    if(paintType == GRADIENTPAINT) {
        p->setBrush(gradient);
    } else if(paintType == FLATPAINT) {
        p->setBrush(paintColor);
    } else {
        p->setBrush(Qt::NoBrush);
    }
}

void UpdatePaintSettings::applyPainterSettingsSk(SkPaint *paint) {
    if(paintType == GRADIENTPAINT) {
        //p->setBrush(gradient);
        paint->setShader(gradientSk);
    } else if(paintType == FLATPAINT) {
        paint->setColor(SkColorSetARGBInline(paintColor.alpha(),
                                             paintColor.red(),
                                             paintColor.green(),
                                             paintColor.blue()));
    } else {
        paint->setColor(SkColorSetARGBInline(0, 0, 0, 0));
    }
}

void UpdatePaintSettings::updateGradient(const QGradientStops &stops, const QPointF &start, const QPointF &finalStop) {
    gradient.setStops(stops);
    gradient.setStart(start);
    gradient.setFinalStop(finalStop);

    int nStops = stops.count();
    SkPoint gradPoints[nStops];
    SkColor gradColors[nStops];
    SkScalar gradPos[nStops];
    SkScalar xInc = finalStop.x() - start.x();
    SkScalar yInc = finalStop.y() - start.y();
    SkScalar currX = start.x();
    SkScalar currY = start.y();
    SkScalar currT = 0.f;
    SkScalar tInc = 1.f/(nStops - 1);

    for(int i = 0; i < nStops; i++) {
        const QGradientStop &stopT = stops.at(i);
        QColor col = stopT.second;
        gradPoints[i] = SkPoint::Make(currX, currY);
        gradColors[i] = SkColorSetARGBInline(col.alpha(),
                                             col.red(),
                                             col.green(),
                                             col.blue());
        gradPos[i] = currT;

        currX += xInc;
        currY += yInc;
        currT += tInc;
    }
    gradientSk = SkGradientShader::MakeLinear(gradPoints,
                                              gradColors,
                                              gradPos,
                                              nStops,
                                              SkShader::kClamp_TileMode);
}

UpdateStrokeSettings::UpdateStrokeSettings(const QColor &paintColorT, const PaintType &paintTypeT, const QLinearGradient &gradientT, const QPainter::CompositionMode &outlineCompositionModeT) :
    UpdatePaintSettings(paintColorT, paintTypeT, gradientT) {
    outlineCompositionMode = outlineCompositionModeT;
}

UpdateStrokeSettings::UpdateStrokeSettings() {}

void UpdateStrokeSettings::applyPainterSettings(QPainter *p) {
    UpdatePaintSettings::applyPainterSettings(p);
    p->setCompositionMode(outlineCompositionMode);
}

void UpdateStrokeSettings::applyPainterSettingsSk(SkPaint *paint) {
    UpdatePaintSettings::applyPainterSettingsSk(paint);
    //canvas->setCompositionMode(outlineCompositionMode);
}
