#include "fillstrokesettings.h"
#include "Colors/ColorWidgets/gradientwidget.h"
#include "mainwindow.h"
#include "undoredo.h"
#include "canvas.h"

Gradient::Gradient(Color color1, Color color2, GradientWidget *gradientWidget, MainWindow *parent) :
    ConnectedToMainWindow(parent)
{
    mGradientWidget = gradientWidget;
    colors << color1;
    colors << color2;
    updateQGradientStops();
}

Gradient::Gradient(Gradient *fromGradient, GradientWidget *gradientWidget, MainWindow *parent) :
    ConnectedToMainWindow(parent)
{
    mGradientWidget = gradientWidget;
    foreach (Color color, fromGradient->colors) {
        colors << color;
    }
    updateQGradientStops();
}

Gradient::Gradient(int sqlIdT, GradientWidget *gradientWidget, MainWindow *parent) :
    ConnectedToMainWindow(parent) {
    QSqlQuery query;
    mGradientWidget = gradientWidget;
    sqlId = sqlIdT;
    QString queryStr = QString("SELECT * FROM gradient WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        queryStr = QString("SELECT colorid FROM gradientcolor WHERE gradientid = %1 ORDER BY positioningradient ASC").
                arg(sqlId);
        if(query.exec(queryStr) ) {
            int idColorId = query.record().indexOf("colorid");
            while(query.next()) {
                int colorId = query.value(idColorId).toInt();
                colors << Color(colorId);
            }
        } else {
            qDebug() << "Could not load gradientcolors for gradient with id " << sqlId;
        }
    } else {
        qDebug() << "Could not load gradient with id " << sqlId;
    }
    updateQGradientStops();
}

int Gradient::saveToSql() {
    QSqlQuery query;
    query.exec("INSERT INTO gradient DEFAULT VALUES");
    sqlId = query.lastInsertId().toInt();
    int posInGradient = 0;
    foreach(Color color, colors) {
        int colorId = color.saveToSql();
        query.exec(QString("INSERT INTO gradientcolor (colorid, gradientid, positioningradient) "
                            "VALUES (%1, %2, %3)").
                    arg(colorId).
                    arg(sqlId).
                    arg(posInGradient) );
        posInGradient++;
    }
    return sqlId;
}

void Gradient::saveToSqlIfPathSelected() {
    foreach(VectorPath *path, mAffectedPaths) {
        BoundingBox *parent = (BoundingBox *) path;
        while(parent != NULL) {
            if(parent->isSelected()) {
                saveToSql();
                return;
            }
            parent = parent->getParent();
        }
    }
}

void Gradient::swapColors(int id1, int id2) {
    colors.swap(id1, id2);
    updateQGradientStops();
}

void Gradient::removeColor(int id) {
    colors.removeAt(id);
    updateQGradientStops();
}

void Gradient::addColor(Color color) {
    colors << color;
    updateQGradientStops();
}

void Gradient::replaceColor(int id, Color color) {
    colors.replace(id, color);
    updateQGradientStops();
}

void Gradient::setColors(QList<Color> newColors)
{
    colors = newColors;
    updateQGradientStops();
    mGradientWidget->repaint();
    mMainWindow->getFillStrokeSettings()->
            setCurrentColor(mGradientWidget->getCurrentColor());
}

void Gradient::startTransform() {
    if(transformPending) return;
    transformPending = true;
    savedColors = colors;
}

bool Gradient::isInPaths(VectorPath *path) {
    return mAffectedPaths.contains(path);
}

void Gradient::addPath(VectorPath *path) {
    mAffectedPaths << path;
}

void Gradient::removePath(VectorPath *path) {
    mAffectedPaths.removeOne(path);
}

bool Gradient::affectsPaths() {
    return !mAffectedPaths.isEmpty();
}

void Gradient::updatePaths()
{
    foreach (VectorPath *path, mAffectedPaths) {
        path->updateDrawGradients();
    }
}

void Gradient::finishTransform()
{
    if(transformPending) {
        transformPending = false;
        addUndoRedo(new ChangeGradientColorsUndoRedo(savedColors, colors, this));
        scheduleRepaint();
        callUpdateSchedulers();
    }
}

void Gradient::updateQGradientStops() {
    qgradientStops.clear();
    qreal inc = 1.f/(colors.length() - 1.f);
    qreal cPos = 0.f;
    for(int i = 0; i < colors.length(); i++) {
        qgradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0.f, 1.f), colors.at(i).qcol) );
        cPos += inc;
    }
    updatePaths();
}

int Gradient::getSqlId() {
    return sqlId;
}


PaintSettings::PaintSettings() {
    color.setCurrentValue(QColor(Qt::white));
}

PaintSettings::PaintSettings(Color colorT, PaintType paintTypeT, Gradient *gradientT) {
    color.setCurrentValue(colorT);
    paintType = paintTypeT;
    gradient = gradientT;
}

PaintSettings::PaintSettings(int sqlId, GradientWidget *gradientWidget) {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM paintsettings WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idPaintType = query.record().indexOf("painttype");
        paintType = static_cast<PaintType>(query.value(idPaintType).toInt());
        int idColorId = query.record().indexOf("colorid");
        color.setCurrentValue(Color(query.value(idColorId).toInt() ) );
        int idGradientId = query.record().indexOf("gradientid");
        if(!query.value(idGradientId).isNull()) {
            gradient = gradientWidget->getGradientBySqlId(
                        query.value(idGradientId).toInt());
        }
    } else {
        qDebug() << "Could not load paintSettings with id " << sqlId;
    }
}

int PaintSettings::saveToSql() {
    QSqlQuery query;
    int colorId = color.getCurrentValue().saveToSql();
    QString gradientId = (gradient == NULL) ? "NULL" : QString::number(gradient->getSqlId());
    query.exec(QString("INSERT INTO paintsettings (painttype, colorid, gradientid) "
                        "VALUES (%1, %2, %3)").
                arg(paintType).
                arg(colorId).
                arg(gradientId) );
    return query.lastInsertId().toInt();
}

StrokeSettings::StrokeSettings() : PaintSettings() {
    color.setCurrentValue(QColor(Qt::black));
    mLineWidth.setCurrentValue(1.);
}

StrokeSettings::StrokeSettings(Color colorT, PaintType paintTypeT, Gradient *gradientT) : PaintSettings(colorT,
                                                                                                        paintTypeT,
                                                                                                        gradientT)
{
    mLineWidth.setCurrentValue(1.);
}

StrokeSettings StrokeSettings::createStrokeSettingsFromSql(int strokeSqlId,
                        GradientWidget *gradientWidget) {
    QSqlQuery query;
    QString queryStr = QString("SELECT paintsettingsid FROM strokesettings WHERE id = %1").
            arg(strokeSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idPaintSettingsId = query.record().indexOf("paintsettingsid");
        int paintSettingsId = static_cast<PaintType>(query.value(idPaintSettingsId).toInt());
        return StrokeSettings(strokeSqlId, paintSettingsId, gradientWidget);
    }
    qDebug() << "Could not load strokesettings with id " << strokeSqlId;
    return StrokeSettings();
}

StrokeSettings::StrokeSettings(int strokeSqlId, int paintSqlId,
                               GradientWidget *gradientWidget) :
    PaintSettings(paintSqlId, gradientWidget) {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM strokesettings WHERE id = %1").
            arg(strokeSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idLineWidth = query.record().indexOf("linewidth");
        int idCapStyle = query.record().indexOf("capstyle");
        int idJoinStyle = query.record().indexOf("joinstyle");
        mLineWidth.setCurrentValue(query.value(idLineWidth).toReal() );
        mCapStyle = static_cast<Qt::PenCapStyle>(query.value(idCapStyle).toInt());
        mJoinStyle = static_cast<Qt::PenJoinStyle>(query.value(idJoinStyle).toInt());
    } else {
        qDebug() << "Could not load strokesettings with id " << strokeSqlId;
    }
}

int StrokeSettings::saveToSql() {
    QSqlQuery query;
    int paintSettingsId = PaintSettings::saveToSql();
    query.exec(QString("INSERT INTO strokesettings (linewidth, capstyle, joinstyle, paintsettingsid) "
                        "VALUES (%1, %2, %3, %4)").
                arg(mLineWidth.getCurrentValue(), 0, 'f').
                arg(mCapStyle).
                arg(mJoinStyle).
                arg(paintSettingsId) );
    return query.lastInsertId().toInt();
}

void StrokeSettings::setLineWidth(qreal newWidth) {
    mLineWidth.setCurrentValue(newWidth);
}

qreal StrokeSettings::lineWidth() {
    return mLineWidth.getCurrentValue();
}

void StrokeSettings::setCapStyle(Qt::PenCapStyle capStyle) {
    mCapStyle = capStyle;
}

Qt::PenCapStyle StrokeSettings::capStyle() {
    return mCapStyle;
}

void StrokeSettings::setJoinStyle(Qt::PenJoinStyle joinStyle) {
    mJoinStyle = joinStyle;
}

Qt::PenJoinStyle StrokeSettings::joinStyle() {
    return mJoinStyle;
}

void StrokeSettings::setStrokerSettings(QPainterPathStroker *stroker) {
    stroker->setWidth(mLineWidth.getCurrentValue());
    stroker->setCapStyle(mCapStyle);
    stroker->setJoinStyle(mJoinStyle);
}

FillStrokeSettingsWidget::FillStrokeSettingsWidget(MainWindow *parent) : QWidget(parent)
{
    //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mMainWindow = parent;
    mUndoRedoSaveTimer = new QTimer(this);
    connect(mUndoRedoSaveTimer, SIGNAL(timeout()),
            this, SLOT(finishTransform()) );

    mGradientWidget = new GradientWidget(this, mMainWindow);
    mColorTypeBar = new QTabBar(this);
    mStrokeSettingsWidget = new QWidget(this);
    mColorsSettingsWidget = new ColorSettingsWidget(this);
    mFillTargetButton = new QPushButton(
                QIcon("pixmaps/icons/ink_properties_fill.png"),
                "Fill", this);
    mStrokeTargetButton = new QPushButton(
                QIcon("pixmaps/icons/ink_properties_stroke_paint.png"),
                "Stroke", this);
    setLayout(mMainLayout);
    mMainLayout->setAlignment(Qt::AlignTop);

    mColorTypeBar->addTab(QIcon("pixmaps/icons/ink_fill_none.png"), "No");
    mColorTypeBar->addTab(QIcon("pixmaps/icons/ink_fill_solid.png"), "Flat");
    mColorTypeBar->addTab(QIcon("pixmaps/icons/ink_fill_gradient.png"),"Gradient");

    mFillTargetButton->setCheckable(true);
    mStrokeTargetButton->setCheckable(true);
    mTargetLayout->addWidget(mFillTargetButton);
    mTargetLayout->addWidget(mStrokeTargetButton);

    mMainLayout->addLayout(mTargetLayout);
    mMainLayout->addWidget(mColorTypeBar);
    mMainLayout->addWidget(mStrokeSettingsWidget);
    mMainLayout->addWidget(mGradientWidget);
    mMainLayout->addWidget(mColorsSettingsWidget);


    mBevelJoinStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_join_bevel.png"), "", this);
    mMiterJointStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_join_miter.png"), "", this);
    mRoundJoinStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_join_round.png"), "", this);
    mBevelJoinStyleButton->setCheckable(true);
    mMiterJointStyleButton->setCheckable(true);
    mRoundJoinStyleButton->setCheckable(true);
    connect(mBevelJoinStyleButton, SIGNAL(released()),
            this, SLOT(setBevelJoinStyle()) );
    connect(mMiterJointStyleButton, SIGNAL(released()),
            this, SLOT(setMiterJoinStyle()) );
    connect(mRoundJoinStyleButton, SIGNAL(released()),
            this, SLOT(setRoundJoinStyle()) );
    mJoinStyleLayout->addWidget(mBevelJoinStyleButton);
    mJoinStyleLayout->addWidget(mMiterJointStyleButton);
    mJoinStyleLayout->addWidget(mRoundJoinStyleButton);

    mStrokeSettingsLayout->addLayout(mJoinStyleLayout);

    mFlatCapStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_cap_flat.png"), "", this);
    mSquareCapStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_cap_square.png"), "", this);
    mRoundCapStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_cap_round.png"), "", this);
    mFlatCapStyleButton->setCheckable(true);
    mSquareCapStyleButton->setCheckable(true);
    mRoundCapStyleButton->setCheckable(true);
    connect(mFlatCapStyleButton, SIGNAL(released()),
            this, SLOT(setFlatCapStyle()) );
    connect(mSquareCapStyleButton, SIGNAL(released()),
            this, SLOT(setSquareCapStyle()) );
    connect(mRoundCapStyleButton, SIGNAL(released()),
            this, SLOT(setRoundCapStyle()) );
    mCapStyleLayout->addWidget(mFlatCapStyleButton);
    mCapStyleLayout->addWidget(mSquareCapStyleButton);
    mCapStyleLayout->addWidget(mRoundCapStyleButton);

    mStrokeSettingsLayout->addLayout(mCapStyleLayout);

    mLineWidthSpin = new QDoubleSpinBox(this);
    mLineWidthSpin->setRange(0.0, 1000.0);
    mLineWidthSpin->setSuffix(" px");
    mLineWidthSpin->setSingleStep(0.1);
    mLineWidthLayout->addWidget(mLineWidthLabel);
    mLineWidthLayout->addWidget(mLineWidthSpin);

    mStrokeSettingsLayout->addLayout(mLineWidthLayout);

    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));

    mStrokeSettingsWidget->setLayout(mStrokeSettingsLayout);

    connect(mFillTargetButton, SIGNAL(released()),
            this, SLOT(setFillTarget()) );
    connect(mStrokeTargetButton, SIGNAL(released()),
            this, SLOT(setStrokeTarget()) );
    connect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );

    connect(mColorsSettingsWidget,
                SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
                this, SLOT(colorChangedTMP(GLfloat,GLfloat,GLfloat,GLfloat)) );

    mFillPickerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_fill_dropper.png"), "", this);
    mFillPickerButton->setSizePolicy(QSizePolicy::Maximum,
                                     QSizePolicy::Maximum);
    mStrokePickerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_stroke_dropper.png"), "", this);
    mStrokePickerButton->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    mFillStrokePickerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_fill_stroke_dropper.png"), "", this);
    mFillStrokePickerButton->setSizePolicy(QSizePolicy::Maximum,
                                           QSizePolicy::Maximum);
    mPickersLayout->addWidget(mFillPickerButton);
    connect(mFillPickerButton, SIGNAL(pressed()),
            this, SLOT(startLoadingFillFromPath()) );
    mPickersLayout->addWidget(mStrokePickerButton);
    connect(mStrokePickerButton, SIGNAL(pressed()),
            this, SLOT(startLoadingStrokeFromPath()) );
    mPickersLayout->addWidget(mFillStrokePickerButton);
    connect(mFillStrokePickerButton, SIGNAL(pressed()),
            this, SLOT(startLoadingSettingsFromPath()) );
    mMainLayout->addLayout(mPickersLayout);

    setFillTarget();
    setCapStyle(Qt::RoundCap);
    setJoinStyle(Qt::RoundJoin);
}

void FillStrokeSettingsWidget::saveGradientsToSqlIfPathSelected() {
    mGradientWidget->saveGradientsToSqlIfPathSelected();
}

void FillStrokeSettingsWidget::setCurrentDisplayedSettings(PaintSettings *settings) {
    if(settings->paintType == GRADIENTPAINT) {
        mGradientWidget->setCurrentGradient(settings->gradient);
    }
    setCurrentPaintType(settings->paintType);
    mColorTypeBar->setCurrentIndex(settings->paintType);
}

void FillStrokeSettingsWidget::setCurrentPaintType(PaintType paintType)
{
    if(paintType == NOPAINT) {
        setNoPaintType();
    } else if (paintType == FLATPAINT) {
        setFlatPaintType();
    } else {
        setGradientPaintType();
    }
}

void FillStrokeSettingsWidget::setStrokeWidth(qreal width)
{
    startTransform();
    mStrokePaintSettings.setLineWidth(width);
    emitTargetSettingsChangedTMP();
}

void FillStrokeSettingsWidget::setCurrentSettings(PaintSettings fillPaintSettings,
                                            StrokeSettings strokePaintSettings)
{
    disconnect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );
    disconnect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
    mFillPaintSettings = fillPaintSettings;
    mStrokePaintSettings = strokePaintSettings;
    mLineWidthSpin->setValue(mStrokePaintSettings.lineWidth());
    if(mTargetId == 0) { // fill
        setCurrentDisplayedSettings(&mFillPaintSettings);
    } else {
        setCurrentDisplayedSettings(&mStrokePaintSettings);
    }

    setCapStyle(strokePaintSettings.capStyle());
    setJoinStyle(strokePaintSettings.joinStyle());
    connect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );
    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
}

void FillStrokeSettingsWidget::setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a) {
    mColorsSettingsWidget->setCurrentColor(h, s, v, a);
}

void FillStrokeSettingsWidget::setCurrentColor(Color color) {
    mColorsSettingsWidget->setCurrentColor(color);
}

void FillStrokeSettingsWidget::saveGradientsToQuery()
{
    mGradientWidget->saveGradientsToQuery();
}

void FillStrokeSettingsWidget::loadAllGradientsFromSql() {
    mGradientWidget->loadAllGradientsFromSql();
}

GradientWidget *FillStrokeSettingsWidget::getGradientWidget() {
    return mGradientWidget;
}

void FillStrokeSettingsWidget::clearAll()
{
    mGradientWidget->clearAll();
}

void FillStrokeSettingsWidget::colorTypeSet(int id)
{
    if(id == 0) {
        setNoPaintType();
    } else if(id == 1) {
        setFlatPaintType();
    } else {
        setGradientPaintType();
    }
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::colorChangedTMP(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    startTransform();
    if(getCurrentTargetPaintSettings()->paintType == FLATPAINT) {
        flatColorSet(h, s, v, a);
    } else if(getCurrentTargetPaintSettings()->paintType == GRADIENTPAINT) {
        mGradientWidget->setCurrentColor(h, s, v, a);
    }
    emitTargetSettingsChangedTMP();
}

void FillStrokeSettingsWidget::flatColorSet(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    Color newColor;
    newColor.setHSV(h, s, v, a);
    getCurrentTargetPaintSettings()->color.setCurrentValue(newColor);
}

void FillStrokeSettingsWidget::connectGradient()
{
    connect(mGradientWidget,
            SIGNAL(selectedColorChanged(GLfloat,GLfloat,GLfloat,GLfloat)),
            mColorsSettingsWidget,
            SLOT(setCurrentColor(GLfloat,GLfloat,GLfloat,GLfloat) ) );
    connect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );
}

void FillStrokeSettingsWidget::disconnectGradient()
{
    disconnect(mGradientWidget,
            SIGNAL(selectedColorChanged(GLfloat,GLfloat,GLfloat,GLfloat)),
            mColorsSettingsWidget,
            SLOT(setCurrentColor(GLfloat,GLfloat,GLfloat,GLfloat) ) );
    disconnect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );
}

void FillStrokeSettingsWidget::setJoinStyle(Qt::PenJoinStyle joinStyle)
{
    mStrokePaintSettings.setJoinStyle(joinStyle);
    mBevelJoinStyleButton->setChecked(joinStyle == Qt::BevelJoin);
    mMiterJointStyleButton->setChecked(joinStyle == Qt::MiterJoin);
    mRoundJoinStyleButton->setChecked(joinStyle == Qt::RoundJoin);
}

void FillStrokeSettingsWidget::setCapStyle(Qt::PenCapStyle capStyle)
{
    mStrokePaintSettings.setCapStyle(capStyle);
    mFlatCapStyleButton->setChecked(capStyle == Qt::FlatCap);
    mSquareCapStyleButton->setChecked(capStyle == Qt::SquareCap);
    mRoundCapStyleButton->setChecked(capStyle == Qt::RoundCap);
}

PaintSettings *FillStrokeSettingsWidget::getCurrentTargetPaintSettings()
{
    if(mTargetId == 0) {
        return &mFillPaintSettings;
    } else {
        return &mStrokePaintSettings;
    }
}

void FillStrokeSettingsWidget::loadSettingsFromPath(VectorPath *path) {
    if(mLoadFillFromPath) {
        mLoadFillFromPath = false;
        mFillPaintSettings = path->getFillSettings();
        emit fillSettingsChanged(mFillPaintSettings, true);
    }
    if(mLoadStrokeFromPath) {
        mLoadStrokeFromPath = false;
        mStrokePaintSettings = path->getStrokeSettings();
        emit strokeSettingsChanged(mStrokePaintSettings, true);
    }
}

void FillStrokeSettingsWidget::emitTargetSettingsChanged()
{
    if(mTargetId == 0) {
        emit fillSettingsChanged(mFillPaintSettings, true);
    } else {
        emit strokeSettingsChanged(mStrokePaintSettings, true);
    }
}

void FillStrokeSettingsWidget::emitTargetSettingsChangedTMP()
{
    if(mTargetId == 0) {
        emit fillSettingsChanged(mFillPaintSettings, false);
    } else {
        emit strokeSettingsChanged(mStrokePaintSettings, false);
    }
}

void FillStrokeSettingsWidget::startLoadingFillFromPath()
{
    mLoadFillFromPath = true;
    mMainWindow->getCanvas()->pickPathForSettings();
}

void FillStrokeSettingsWidget::startLoadingStrokeFromPath()
{
    mLoadStrokeFromPath = true;
    mMainWindow->getCanvas()->pickPathForSettings();
}

void FillStrokeSettingsWidget::startLoadingSettingsFromPath()
{
    mLoadFillFromPath = true;
    mLoadStrokeFromPath = true;
    mMainWindow->getCanvas()->pickPathForSettings();
}

void FillStrokeSettingsWidget::finishTransform()
{
    if(mTransormStarted) {
        mTransormStarted = false;
        if(getCurrentTargetPaintSettings()->paintType == GRADIENTPAINT) {
            mGradientWidget->finishGradientTransform();
        } else {
            if(mTargetId == 0) {
                emit finishFillSettingsTransform();
            } else {
                emit finishStrokeSettingsTransform();
            }
        }
    }
}

void FillStrokeSettingsWidget::startTransform()
{
    waitToSaveChanges();
    if(mTransormStarted) return;
    mTransormStarted = true;
    if(getCurrentTargetPaintSettings()->paintType == GRADIENTPAINT) {
        mGradientWidget->startGradientTransform();
    } else {
        if(mTargetId == 0) {
            emit startFillSettingsTransform();
        } else {
            emit startStrokeSettingsTransform();
        }
    }
}

void FillStrokeSettingsWidget::setGradient(Gradient *gradient)
{
    getCurrentTargetPaintSettings()->gradient = gradient;
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setBevelJoinStyle()
{
    setJoinStyle(Qt::BevelJoin);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setMiterJoinStyle()
{
    setJoinStyle(Qt::MiterJoin);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setRoundJoinStyle()
{
    setJoinStyle(Qt::RoundJoin);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setFlatCapStyle()
{
    setCapStyle(Qt::FlatCap);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setSquareCapStyle()
{
    setCapStyle(Qt::SquareCap);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setRoundCapStyle()
{
    setCapStyle(Qt::RoundCap);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::waitToSaveChanges()
{
    if(mUndoRedoSaveTimer->isActive()) {
        mUndoRedoSaveTimer->setInterval(50);
        return;
    }
    mUndoRedoSaveTimer->stop();
    mUndoRedoSaveTimer->setInterval(50);
    mUndoRedoSaveTimer->setSingleShot(true);
    mUndoRedoSaveTimer->start();
}

void FillStrokeSettingsWidget::setFillTarget()
{
    mTargetId = 0;
    mFillTargetButton->setChecked(true);
    mStrokeTargetButton->setChecked(false);
    mStrokeSettingsWidget->hide();
    setCurrentDisplayedSettings(&mFillPaintSettings);
}

void FillStrokeSettingsWidget::setStrokeTarget()
{
    mTargetId = 1;
    mStrokeTargetButton->setChecked(true);
    mFillTargetButton->setChecked(false);
    mStrokeSettingsWidget->show();
    setCurrentDisplayedSettings(&mStrokePaintSettings);
}

void FillStrokeSettingsWidget::setNoPaintType()
{
    getCurrentTargetPaintSettings()->paintType = NOPAINT;
    mColorsSettingsWidget->hide();
    mGradientWidget->hide();
}

void FillStrokeSettingsWidget::setFlatPaintType()
{
    disconnectGradient();
    mColorsSettingsWidget->show();
    mGradientWidget->hide();
    mColorsSettingsWidget->setCurrentColor(
                getCurrentTargetPaintSettings()->color.getCurrentValue());
    getCurrentTargetPaintSettings()->paintType = FLATPAINT;
}

void FillStrokeSettingsWidget::setGradientPaintType()
{
    connectGradient();
    if(getCurrentTargetPaintSettings()->gradient == NULL) {
        getCurrentTargetPaintSettings()->gradient = mGradientWidget->getCurrentGradient();
    }
    mColorsSettingsWidget->show();
    mGradientWidget->show();
    mColorsSettingsWidget->setCurrentColor(mGradientWidget->getCurrentColor());

    getCurrentTargetPaintSettings()->paintType = GRADIENTPAINT;

    mGradientWidget->update();
}
