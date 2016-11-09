#include "coloranimator.h"
#include "Colors/helpers.h"
#include "boxeslist.h"
#include <QDebug>

ColorAnimator::ColorAnimator() : ComplexAnimator()
{
    setName("color");
    mAlphaAnimator.setName("alpha");
    setColorMode(RGBMODE);

    addChildAnimator(&mVal1Animator);
    addChildAnimator(&mVal2Animator);
    addChildAnimator(&mVal3Animator);
    addChildAnimator(&mAlphaAnimator);

    mVal1Animator.setValueRange(0., 1.);
    mVal1Animator.setPrefferedValueStep(0.05);
    mVal1Animator.freezeMinMaxValues();
    mVal2Animator.setValueRange(0., 1.);
    mVal2Animator.setPrefferedValueStep(0.05);
    mVal2Animator.freezeMinMaxValues();
    mVal3Animator.setValueRange(0., 1.);
    mVal3Animator.setPrefferedValueStep(0.05);
    mVal3Animator.freezeMinMaxValues();
    mAlphaAnimator.setValueRange(0., 1.);
    mAlphaAnimator.setPrefferedValueStep(0.05);
    mAlphaAnimator.freezeMinMaxValues();

    mVal1Animator.blockPointer();
    mVal2Animator.blockPointer();
    mVal3Animator.blockPointer();
    mAlphaAnimator.blockPointer();
}

void ColorAnimator::loadFromSql(int sqlId)
{
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM coloranimator WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idModeAnimator = query.record().indexOf("colormode");
        ColorMode colorMode = static_cast<ColorMode>(query.value(idModeAnimator).toInt());
        setColorMode(colorMode);
        int idVal1AnimatorId = query.record().indexOf("val1animatorid");
        mVal1Animator.loadFromSql(query.value(idVal1AnimatorId).toInt());
        int idVal2AnimatorId = query.record().indexOf("val2animatorid");
        mVal2Animator.loadFromSql(query.value(idVal2AnimatorId).toInt());
        int idVal3AnimatorId = query.record().indexOf("val3animatorid");
        mVal3Animator.loadFromSql(query.value(idVal3AnimatorId).toInt());
        int idAlphaAnimatorId = query.record().indexOf("alphaanimatorid");
        mAlphaAnimator.loadFromSql(query.value(idAlphaAnimatorId).toInt());
    } else {
        qDebug() << "Could not load color with id " << sqlId;
    }
}

#include <QSqlError>
int ColorAnimator::saveToSql()
{
    int val1AnimatorId = mVal1Animator.saveToSql();
    int val2AnimatorId = mVal2Animator.saveToSql();
    int val3AnimatorId = mVal3Animator.saveToSql();
    int alphaAnimatorId = mAlphaAnimator.saveToSql();
    QSqlQuery query;
    if(!query.exec(QString("INSERT INTO coloranimator (colormode, val1animatorid, "
                       "val2animatorid, val3animatorid, alphaanimatorid) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(mColorMode).
                arg(val1AnimatorId).
                arg(val2AnimatorId).
                arg(val3AnimatorId).
                arg(alphaAnimatorId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
    return query.lastInsertId().toInt();
}

void ColorAnimator::setCurrentValue(Color colorValue, bool finish)
{
    if(mColorMode == RGBMODE) {
        mVal1Animator.setCurrentValue(colorValue.gl_r, finish);
        mVal2Animator.setCurrentValue(colorValue.gl_g, finish);
        mVal3Animator.setCurrentValue(colorValue.gl_b, finish);
    } else if(mColorMode == HSVMODE) {
        mVal1Animator.setCurrentValue(colorValue.gl_h, finish);
        mVal2Animator.setCurrentValue(colorValue.gl_s, finish);
        mVal3Animator.setCurrentValue(colorValue.gl_v, finish);
    } else { // HSLMODE
        float h = colorValue.gl_h;
        float s = colorValue.gl_s;
        float l = colorValue.gl_v;
        hsv_to_hsl(&h, &s, &l);

        mVal1Animator.setCurrentValue(h, finish);
        mVal2Animator.setCurrentValue(s, finish);
        mVal3Animator.setCurrentValue(l, finish);
    }
    mAlphaAnimator.setCurrentValue(colorValue.gl_a, finish);
}

void ColorAnimator::setCurrentValue(QColor qcolorValue, bool finish)
{
    Color color;
    color.setQColor(qcolorValue);
    setCurrentValue(color, finish);
}

Color ColorAnimator::getCurrentValue() const
{
    Color color;
    if(mColorMode == RGBMODE) {
        color.setRGB(mVal1Animator.getCurrentValue(),
                     mVal2Animator.getCurrentValue(),
                     mVal3Animator.getCurrentValue(),
                     mAlphaAnimator.getCurrentValue() );
    } else if(mColorMode == HSVMODE) {
        color.setHSV(mVal1Animator.getCurrentValue(),
                     mVal2Animator.getCurrentValue(),
                     mVal3Animator.getCurrentValue(),
                     mAlphaAnimator.getCurrentValue() );
    } else { // HSLMODE
        color.setHSL(mVal1Animator.getCurrentValue(),
                     mVal2Animator.getCurrentValue(),
                     mVal3Animator.getCurrentValue(),
                     mAlphaAnimator.getCurrentValue() );
    }
    return color;
}

QrealKey *ColorAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= BoxesList::getListItemHeight()) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                   minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        if(relY <= 2*BoxesList::getListItemHeight()) {
            return mVal1Animator.getKeyAtPos(relX, relY,
                                     minViewedFrame, pixelsPerFrame);
        } else if(relY <= 3*BoxesList::getListItemHeight()) {
            return mVal2Animator.getKeyAtPos(relX, relY,
                                       minViewedFrame, pixelsPerFrame);
        } else if(relY <= 4*BoxesList::getListItemHeight()) {
            return mVal3Animator.getKeyAtPos(relX, relY,
                                     minViewedFrame, pixelsPerFrame);
        } else if(relY <= 5*BoxesList::getListItemHeight()) {
            return mAlphaAnimator.getKeyAtPos(relX, relY,
                                       minViewedFrame, pixelsPerFrame);
        }
    }
    return NULL;
}

void ColorAnimator::setColorMode(ColorMode colorMode)
{
    if(colorMode == RGBMODE) {
        mVal1Animator.setName("red");
        mVal2Animator.setName("green");
        mVal3Animator.setName("blue");
    } else if(colorMode == HSVMODE) {
        mVal1Animator.setName("hue");
        mVal2Animator.setName("saturation");
        mVal3Animator.setName("value");
    } else { // HSLMODE
        mVal1Animator.setName("hue");
        mVal2Animator.setName("saturation");
        mVal3Animator.setName("lightness");
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

    qreal crF = mVal1Animator.getCurrentValue();
    qreal cgF = mVal2Animator.getCurrentValue();
    qreal cbF = mVal3Animator.getCurrentValue();

    foreach(QrealKey *key, mKeys) {
        int frame = key->getFrame();

        qreal rF = mVal1Animator.getValueAtFrame(frame);
        qreal gF = mVal2Animator.getValueAtFrame(frame);
        qreal bF = mVal3Animator.getValueAtFrame(frame);

        foo(&rF, &gF, &bF);

        mVal1Animator.saveValueToKey(frame, rF);
        mVal2Animator.saveValueToKey(frame, gF);
        mVal3Animator.saveValueToKey(frame, bF);
    }

    foo(&crF, &cgF, &cbF);

    mVal1Animator.setCurrentValue(crF);
    mVal2Animator.setCurrentValue(cgF);
    mVal3Animator.setCurrentValue(cbF);

    if(!mKeys.isEmpty()) {
        mVal1Animator.setRecording(true);
        mVal2Animator.setRecording(true);
        mVal3Animator.setRecording(true);
    }

    mColorMode = colorMode;
}

void ColorAnimator::startVal1Transform()
{
    mVal1Animator.startTransform();
}

void ColorAnimator::startVal2Transform()
{
    mVal2Animator.startTransform();
}

void ColorAnimator::startVal3Transform()
{
    mVal3Animator.startTransform();
}

void ColorAnimator::startAlphaTransform()
{
    mAlphaAnimator.startTransform();
}

void ColorAnimator::openContextMenu(QPoint pos) {
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
            if(!mIsRecording) {
                setRecording(true);
            }
            saveCurrentValueAsKey();
        } else if(selected_action == rgbAction) {
            setColorMode(RGBMODE);
        } else if(selected_action == hsvAction) {
            setColorMode(HSVMODE);
        } else if(selected_action == hslAction) {
            setColorMode(HSLMODE);
        }

        callUpdateSchedulers();
    } else {

    }
}
