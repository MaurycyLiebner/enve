#include "Animators/coloranimator.h"
#include "Colors/helpers.h"
#include <QDebug>

ColorAnimator::ColorAnimator() : ComplexAnimator() {
    prp_setName("color");
    mAlphaAnimator->prp_setName("alpha");
    setColorMode(RGBMODE);

    ca_addChildAnimator(mVal1Animator.data());
    ca_addChildAnimator(mVal2Animator.data());
    ca_addChildAnimator(mVal3Animator.data());
    ca_addChildAnimator(mAlphaAnimator.data());

    mVal1Animator->qra_setValueRange(0., 1.);
    mVal1Animator->setPrefferedValueStep(0.05);
    mVal1Animator->freezeMinMaxValues();
    mVal2Animator->qra_setValueRange(0., 1.);
    mVal2Animator->setPrefferedValueStep(0.05);
    mVal2Animator->freezeMinMaxValues();
    mVal3Animator->qra_setValueRange(0., 1.);
    mVal3Animator->setPrefferedValueStep(0.05);
    mVal3Animator->freezeMinMaxValues();
    mAlphaAnimator->qra_setValueRange(0., 1.);
    mAlphaAnimator->setPrefferedValueStep(0.05);
    mAlphaAnimator->freezeMinMaxValues();
}

void ColorAnimator::prp_loadFromSql(const int &sqlId) {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM coloranimator WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idModeAnimator = query.record().indexOf("colormode");
        ColorMode colorMode = static_cast<ColorMode>(
                    query.value(idModeAnimator).toInt());
        setColorMode(colorMode);
        int idVal1AnimatorId = query.record().indexOf("val1animatorid");
        mVal1Animator->prp_loadFromSql(query.value(idVal1AnimatorId).toInt());
        int idVal2AnimatorId = query.record().indexOf("val2animatorid");
        mVal2Animator->prp_loadFromSql(query.value(idVal2AnimatorId).toInt());
        int idVal3AnimatorId = query.record().indexOf("val3animatorid");
        mVal3Animator->prp_loadFromSql(query.value(idVal3AnimatorId).toInt());
        int idAlphaAnimatorId = query.record().indexOf("alphaanimatorid");
        mAlphaAnimator->prp_loadFromSql(query.value(idAlphaAnimatorId).toInt());
    } else {
        qDebug() << "Could not load color with id " << sqlId;
    }
}

#include <QSqlError>
int ColorAnimator::prp_saveToSql(QSqlQuery *query,
                                 const int &parentId) {
    Q_UNUSED(parentId);
    int val1AnimatorId = mVal1Animator->prp_saveToSql(query);
    int val2AnimatorId = mVal2Animator->prp_saveToSql(query);
    int val3AnimatorId = mVal3Animator->prp_saveToSql(query);
    int alphaAnimatorId = mAlphaAnimator->prp_saveToSql(query);
    if(!query->exec(QString("INSERT INTO coloranimator (colormode, val1animatorid, "
                       "val2animatorid, val3animatorid, alphaanimatorid) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(mColorMode).
                arg(val1AnimatorId).
                arg(val2AnimatorId).
                arg(val3AnimatorId).
                arg(alphaAnimatorId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return query->lastInsertId().toInt();
}

void ColorAnimator::qra_setCurrentValue(const Color &colorValue,
                                        const bool &saveUndoRedo,
                                        const bool &finish) {
    if(mColorMode == RGBMODE) {
        mVal1Animator->qra_setCurrentValue(colorValue.gl_r,
                                           saveUndoRedo,
                                           finish);
        mVal2Animator->qra_setCurrentValue(colorValue.gl_g,
                                           saveUndoRedo,
                                           finish);
        mVal3Animator->qra_setCurrentValue(colorValue.gl_b,
                                           saveUndoRedo,
                                           finish);
    } else if(mColorMode == HSVMODE) {
        mVal1Animator->qra_setCurrentValue(colorValue.gl_h,
                                           saveUndoRedo,
                                           finish);
        mVal2Animator->qra_setCurrentValue(colorValue.gl_s,
                                           saveUndoRedo,
                                           finish);
        mVal3Animator->qra_setCurrentValue(colorValue.gl_v,
                                           saveUndoRedo,
                                           finish);
    } else { // HSLMODE
        float h = colorValue.gl_h;
        float s = colorValue.gl_s;
        float l = colorValue.gl_v;
        hsv_to_hsl(&h, &s, &l);

        mVal1Animator->qra_setCurrentValue(h, saveUndoRedo, finish);
        mVal2Animator->qra_setCurrentValue(s, saveUndoRedo, finish);
        mVal3Animator->qra_setCurrentValue(l, saveUndoRedo, finish);
    }
    mAlphaAnimator->qra_setCurrentValue(colorValue.gl_a, saveUndoRedo, finish);
}

void ColorAnimator::qra_setCurrentValue(QColor qcolorValue,
                                        const bool &saveUndoRedo,
                                        const bool &finish) {
    Color color;
    color.setQColor(qcolorValue);
    qra_setCurrentValue(color, saveUndoRedo, finish);
}

Color ColorAnimator::getCurrentColor() const {
    Color color;
    if(mColorMode == RGBMODE) {
        color.setRGB(mVal1Animator->qra_getCurrentValue(),
                     mVal2Animator->qra_getCurrentValue(),
                     mVal3Animator->qra_getCurrentValue(),
                     mAlphaAnimator->qra_getCurrentValue() );
    } else if(mColorMode == HSVMODE) {
        color.setHSV(mVal1Animator->qra_getCurrentValue(),
                     mVal2Animator->qra_getCurrentValue(),
                     mVal3Animator->qra_getCurrentValue(),
                     mAlphaAnimator->qra_getCurrentValue() );
    } else { // HSLMODE
        color.setHSL(mVal1Animator->qra_getCurrentValue(),
                     mVal2Animator->qra_getCurrentValue(),
                     mVal3Animator->qra_getCurrentValue(),
                     mAlphaAnimator->qra_getCurrentValue() );
    }
    return color;
}

void ColorAnimator::setColorMode(ColorMode colorMode) {
    if(colorMode == RGBMODE) {
        mVal1Animator->prp_setName("red");
        mVal2Animator->prp_setName("green");
        mVal3Animator->prp_setName("blue");
    } else if(colorMode == HSVMODE) {
        mVal1Animator->prp_setName("hue");
        mVal2Animator->prp_setName("saturation");
        mVal3Animator->prp_setName("value");
    } else { // HSLMODE
        mVal1Animator->prp_setName("hue");
        mVal2Animator->prp_setName("saturation");
        mVal3Animator->prp_setName("lightness");
    }
    if(mColorMode == colorMode) return;

    void (*foo)(qreal*, qreal*, qreal*);
    if(mColorMode == RGBMODE && colorMode == HSVMODE) {
        foo = &qrgb_to_hsv;
    } else if(mColorMode == RGBMODE && colorMode == HSLMODE) {
        foo = &qrgb_to_hsl;
    } else if(mColorMode == HSVMODE && colorMode == RGBMODE) {
        foo = &qhsv_to_rgb;
    } else if(mColorMode == HSVMODE && colorMode == HSLMODE) {
        foo = &qhsv_to_hsl;
    } else if(mColorMode == HSLMODE && colorMode == RGBMODE) {
        foo = &qhsl_to_rgb;
    } else if(mColorMode == HSLMODE && colorMode == HSVMODE) {
        foo = &qhsl_to_hsv;
    } else {
        return;
    }

    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        int frame = key->getAbsFrame();

        qreal rF = mVal1Animator->qra_getValueAtAbsFrame(frame);
        qreal gF = mVal2Animator->qra_getValueAtAbsFrame(frame);
        qreal bF = mVal3Animator->qra_getValueAtAbsFrame(frame);

        foo(&rF, &gF, &bF);

        mVal1Animator->qra_saveValueToKey(frame, rF);
        mVal2Animator->qra_saveValueToKey(frame, gF);
        mVal3Animator->qra_saveValueToKey(frame, bF);
    }

    if(!anim_mKeys.isEmpty()) {
        mVal1Animator->anim_setRecordingWithoutChangingKeys(true);
        mVal2Animator->anim_setRecordingWithoutChangingKeys(true);
        mVal3Animator->anim_setRecordingWithoutChangingKeys(true);
    } else {
        qreal crF = mVal1Animator->qra_getCurrentValue();
        qreal cgF = mVal2Animator->qra_getCurrentValue();
        qreal cbF = mVal3Animator->qra_getCurrentValue();

        foo(&crF, &cgF, &cbF);

        mVal1Animator->qra_setCurrentValue(crF);
        mVal2Animator->qra_setCurrentValue(cgF);
        mVal3Animator->qra_setCurrentValue(cbF);
    }

    mColorMode = colorMode;

    emit colorModeChanged(mColorMode);
}

void ColorAnimator::startVal1Transform() {
    mVal1Animator->prp_startTransform();
}

void ColorAnimator::startVal2Transform() {
    mVal2Animator->prp_startTransform();
}

void ColorAnimator::startVal3Transform() {
    mVal3Animator->prp_startTransform();
}

void ColorAnimator::startAlphaTransform() {
    mAlphaAnimator->prp_startTransform();
}

void ColorAnimator::setCurrentVal1Value(const qreal &val1,
                                        const bool &finish) {
    mVal1Animator->qra_setCurrentValue(val1, finish);
}

void ColorAnimator::setCurrentVal2Value(const qreal &val2,
                                        const bool &finish) {
    mVal2Animator->qra_setCurrentValue(val2, finish);
}

void ColorAnimator::setCurrentVal3Value(const qreal &val3,
                                        const bool &finish) {
    mVal3Animator->qra_setCurrentValue(val3, finish);
}

void ColorAnimator::setCurrentAlphaValue(const qreal &alpha,
                                         const bool &finish) {
    mAlphaAnimator->qra_setCurrentValue(alpha, finish);
}

void ColorAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) {
        prp_setRecording(true);
    }
    mVal1Animator->anim_saveCurrentValueAsKey();
    mVal2Animator->anim_saveCurrentValueAsKey();
    mVal3Animator->anim_saveCurrentValueAsKey();
}

#include <QMenu>
void ColorAnimator::prp_openContextMenu(QPoint pos) {
    QMenu menu;
    menu.addAction("Add Key");

    QMenu colorModeMenu;
    colorModeMenu.setTitle("Color Mode");

    QAction *rgbAction = new QAction("RGB");
    rgbAction->setCheckable(true);
    rgbAction->setChecked(mColorMode == RGBMODE);

    QAction *hsvAction = new QAction("HSV");
    hsvAction->setCheckable(true);
    hsvAction->setChecked(mColorMode == HSVMODE);

    QAction *hslAction = new QAction("HSL");
    hslAction->setCheckable(true);
    hslAction->setChecked(mColorMode == HSLMODE);

    colorModeMenu.addAction(rgbAction);
    colorModeMenu.addAction(hsvAction);
    colorModeMenu.addAction(hslAction);
    menu.addMenu(&colorModeMenu);
    QAction *selected_action = menu.exec(pos);
    if(selected_action != NULL)
    {
        if(selected_action->text() == "Add Key")
        {
            anim_saveCurrentValueAsKey();
        } else if(selected_action == rgbAction) {
            setColorMode(RGBMODE);
        } else if(selected_action == hsvAction) {
            setColorMode(HSVMODE);
        } else if(selected_action == hslAction) {
            setColorMode(HSLMODE);
        }
    } else {

    }
}

void ColorAnimator::makeDuplicate(Property *target) {
    ColorAnimator *colorTarget = (ColorAnimator*)target;

    colorTarget->duplicateVal1AnimatorFrom(mVal1Animator.data());
    colorTarget->duplicateVal2AnimatorFrom(mVal2Animator.data());
    colorTarget->duplicateVal3AnimatorFrom(mVal3Animator.data());
    colorTarget->duplicateAlphaAnimatorFrom(mAlphaAnimator.data());
}

Property *ColorAnimator::makeDuplicate() {
    ColorAnimator *colorAnimator = new ColorAnimator();
    makeDuplicate(colorAnimator);
    return colorAnimator;
}

void ColorAnimator::duplicateVal1AnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mVal1Animator.data());
}

void ColorAnimator::duplicateVal2AnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mVal2Animator.data());
}

void ColorAnimator::duplicateVal3AnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mVal3Animator.data());
}

void ColorAnimator::duplicateAlphaAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mAlphaAnimator.data());
}
