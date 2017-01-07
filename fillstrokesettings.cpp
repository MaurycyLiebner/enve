#include "fillstrokesettings.h"
#include "Colors/ColorWidgets/gradientwidget.h"
#include "mainwindow.h"
#include "undoredo.h"
#include "canvas.h"
#include "updatescheduler.h"

Gradient::Gradient(Color color1, Color color2, GradientWidget *gradientWidget) :
    ComplexAnimator()
{
    setName("gradient");
    mGradientWidget = gradientWidget;
    addColorToList(color1);
    addColorToList(color2);
    updateQGradientStops();
}

Gradient::Gradient(Gradient *fromGradient, GradientWidget *gradientWidget) :
    ComplexAnimator() {
    setName("gradient");
    mGradientWidget = gradientWidget;
    foreach (ColorAnimator *color, fromGradient->mColors) {
        addColorToList(color->getCurrentValue());
    }
    updateQGradientStops();
}

Gradient::Gradient(int sqlIdT, GradientWidget *gradientWidget) :
    ComplexAnimator() {
    setName("gradient");
    QSqlQuery query;
    mGradientWidget = gradientWidget;
    mSqlId = sqlIdT;
    QString queryStr = QString("SELECT * FROM gradient WHERE id = %1").
            arg(mSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        queryStr = QString("SELECT colorid FROM gradientcolor WHERE gradientid = %1 ORDER BY positioningradient ASC").
                arg(mSqlId);
        if(query.exec(queryStr) ) {
            int idColorId = query.record().indexOf("colorid");
            while(query.next()) {
                int colorId = query.value(idColorId).toInt();
                ColorAnimator *newAnimator = new ColorAnimator();
                newAnimator->loadFromSql(colorId);
                addColorToList(newAnimator);
            }
        } else {
            qDebug() << "Could not load gradientcolors for gradient with id " << mSqlId;
        }
    } else {
        qDebug() << "Could not load gradient with id " << mSqlId;
    }
    updateQGradientStops();
}

void Gradient::startTransform() {
    //savedColors = colors;
}

void Gradient::addColorToList(Color color) {
    ColorAnimator *newColorAnimator = new ColorAnimator();
    newColorAnimator->setCurrentValue(color);
    addColorToList(newColorAnimator);
}

void Gradient::addColorToList(ColorAnimator *newColorAnimator) {
    newColorAnimator->setUpdater(new GradientUpdater(this) );
    mColors << newColorAnimator;

    addChildAnimator(newColorAnimator);
}

Color Gradient::getCurrentColorAt(int id)
{
    return mColors.at(id)->getCurrentValue();
}

int Gradient::getColorCount()
{
    return mColors.length();
}

QColor Gradient::getLastQGradientStopQColor()
{
    return mQGradientStops.last().second;
}

QColor Gradient::getFirstQGradientStopQColor()
{
    return mQGradientStops.first().second;
}

QGradientStops Gradient::getQGradientStops()
{
    return mQGradientStops;
}

int Gradient::saveToSql(QSqlQuery *query) {
    query->exec("INSERT INTO gradient DEFAULT VALUES");
    mSqlId = query->lastInsertId().toInt();
    int posInGradient = 0;
    foreach(ColorAnimator *color, mColors) {
        int colorId = color->saveToSql(query);
        query->exec(QString("INSERT INTO gradientcolor (colorid, gradientid, positioningradient) "
                            "VALUES (%1, %2, %3)").
                    arg(colorId).
                    arg(mSqlId).
                    arg(posInGradient) );
        posInGradient++;
    }
    return mSqlId;
}

void Gradient::saveToSqlIfPathSelected(QSqlQuery *query) {
    foreach(PathBox *path, mAffectedPaths) {
        BoundingBox *parent = (BoundingBox *) path;
        while(parent != NULL) {
            if(parent->isSelected()) {
                saveToSql(query);
                return;
            }
            parent = parent->getParent();
        }
    }
}

void Gradient::swapColors(int id1, int id2) {
    swapChildAnimators(mColors.at(id1), mColors.at(id2));
    mColors.swap(id1, id2);
    updateQGradientStops();
}

void Gradient::removeColor(int id) {
    removeChildAnimator(mColors.at(id) );
    mColors.takeAt(id)->decNumberPointers();
    updateQGradientStops();
}

void Gradient::addColor(Color color) {
    addColorToList(color);
    updateQGradientStops();
}

void Gradient::replaceColor(int id, Color color) {
    mColors.at(id)->setCurrentValue(color);
    updateQGradientStops();
}

void Gradient::setColors(QList<Color> newColors)
{
//    colors = newColors;
//    updateQGradientStops();
//    mGradientWidget->repaint();
//    mMainWindow->getFillStrokeSettings()->
//            setCurrentColor(mGradientWidget->getCurrentColor());
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
    foreach (PathBox *path, mAffectedPaths) {
        path->updateDrawGradients();
        path->scheduleAwaitUpdate();
    }
}

void Gradient::finishTransform() {
    ComplexAnimator::finishTransform();
    //addUndoRedo(new ChangeGradientColorsUndoRedo(savedColors, colors, this));
    //savedColors = colors;
    callUpdateSchedulers();
}

void Gradient::scheduleQGradientStopsUpdate() {
    if(mQGradientStopsUpdateNeeded) return;
    mQGradientStopsUpdateNeeded = true;
    //addUpdateScheduler(new QGradientStopsUpdateScheduler(this) );
}

void Gradient::updateQGradientStopsIfNeeded() {
    if(mQGradientStopsUpdateNeeded) {
        mQGradientStopsUpdateNeeded = false;
        updateQGradientStops();
    }
}

void Gradient::startColorIdTransform(int id) {
    if(mColors.count() <= id || id < 0) return;
    mColors.at(id)->startTransform();
}

void Gradient::updateQGradientStops() {
    mQGradientStops.clear();
    qreal inc = 1./(mColors.length() - 1.);
    qreal cPos = 0.;
    for(int i = 0; i < mColors.length(); i++) {
        mQGradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0., 1.),
                                    mColors.at(i)->getCurrentValue().qcol) );
        cPos += inc;
    }
    updatePaths();
}

int Gradient::getSqlId() {
    return mSqlId;
}

void Gradient::setSqlId(int id) {
    mSqlId = id;
}

PaintSettings::PaintSettings() : PaintSettings(Color(255, 255, 255),
                                               PaintType::FLATPAINT,
                                               NULL) {
}

PaintSettings::PaintSettings(Color colorT,
                             PaintType paintTypeT,
                             Gradient *gradientT) : ComplexAnimator() {
    setName("fill");
    mColor.blockPointer();
    mColor.setCurrentValue(colorT);
    mPaintType = paintTypeT;
    mGradient = gradientT;

    addChildAnimator(&mColor);
}

void PaintSettings::setPaintPathTarget(PathBox *path) {
    mColor.setUpdater(new DisplayedFillStrokeSettingsUpdater(path));
}

void PaintSettings::loadFromSql(int sqlId, GradientWidget *gradientWidget) {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM paintsettings WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idPaintType = query.record().indexOf("painttype");
        mPaintType = static_cast<PaintType>(query.value(idPaintType).toInt());
        int idColorId = query.record().indexOf("colorid");
        mColor.loadFromSql(query.value(idColorId).toInt() );
        int idGradientId = query.record().indexOf("gradientid");
        if(!query.value(idGradientId).isNull()) {
            mGradient = gradientWidget->getGradientBySqlId(
                        query.value(idGradientId).toInt());
        }
    } else {
        qDebug() << "Could not load paintSettings with id " << sqlId;
    }
}

int PaintSettings::saveToSql(QSqlQuery *query) {
    int colorId = mColor.saveToSql(query);
    QString gradientId = (mGradient == NULL) ? "NULL" :
                                               QString::number(
                                                   mGradient->getSqlId());
    query->exec(QString("INSERT INTO paintsettings (painttype, colorid, gradientid) "
                        "VALUES (%1, %2, %3)").
                arg(mPaintType).
                arg(colorId).
                arg(gradientId) );
    return query->lastInsertId().toInt();
}

StrokeSettings::StrokeSettings() : StrokeSettings(Color(0, 0, 0),
                                                  PaintType::FLATPAINT,
                                                  NULL) {
}

StrokeSettings::StrokeSettings(Color colorT,
                               PaintType paintTypeT,
                               Gradient *gradientT) : PaintSettings(colorT,
                                                                    paintTypeT,
                                                                    gradientT) {
    setName("stroke");
    mLineWidth.setCurrentValue(1.);
    mLineWidth.setName("thickness");

    addChildAnimator(&mLineWidth);
    mLineWidth.blockPointer();

    mLineWidth.setValueRange(0., mLineWidth.getMaxPossibleValue());
}

void StrokeSettings::setLineWidthUpdaterTarget(PathBox *path) {
    setUpdater(new StrokeWidthUpdater(path));
    setPaintPathTarget(path);
}

void StrokeSettings::loadFromSql(int strokeSqlId, GradientWidget *gradientWidget) {
    QSqlQuery query;
    QString queryStr = QString("SELECT paintsettingsid FROM strokesettings WHERE id = %1").
            arg(strokeSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idPaintSettingsId = query.record().indexOf("paintsettingsid");
        int paintSettingsId = static_cast<PaintType>(query.value(idPaintSettingsId).toInt());
        loadFromSql(strokeSqlId, paintSettingsId, gradientWidget);
    } else {
        qDebug() << "Could not load strokesettings with id " << strokeSqlId;
    }
}

void StrokeSettings::loadFromSql(int strokeSqlId, int paintSqlId,
                               GradientWidget *gradientWidget) {
    PaintSettings::loadFromSql(paintSqlId, gradientWidget);
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM strokesettings WHERE id = %1").
            arg(strokeSqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idLineWidth = query.record().indexOf("linewidthanimatorid");
        int idCapStyle = query.record().indexOf("capstyle");
        int idJoinStyle = query.record().indexOf("joinstyle");
        mLineWidth.loadFromSql(query.value(idLineWidth).toInt() );
        mCapStyle = static_cast<Qt::PenCapStyle>(query.value(idCapStyle).toInt());
        mJoinStyle = static_cast<Qt::PenJoinStyle>(query.value(idJoinStyle).toInt());
    } else {
        qDebug() << "Could not load strokesettings with id " << strokeSqlId;
    }
}

int StrokeSettings::saveToSql(QSqlQuery *query) {
    int paintSettingsId = PaintSettings::saveToSql(query);
    int lineWidthId = mLineWidth.saveToSql(query);
    query->exec(QString("INSERT INTO strokesettings (linewidthanimatorid, "
                       "capstyle, joinstyle, paintsettingsid) "
                       "VALUES (%1, %2, %3, %4)").
                arg(lineWidthId).
                arg(mCapStyle).
                arg(mJoinStyle).
                arg(paintSettingsId) );
    return query->lastInsertId().toInt();
}

void StrokeSettings::setCurrentStrokeWidth(qreal newWidth) {
    mLineWidth.setCurrentValue(newWidth);
}

void StrokeSettings::setCapStyle(Qt::PenCapStyle capStyle) {
    mCapStyle = capStyle;
}

void StrokeSettings::setJoinStyle(Qt::PenJoinStyle joinStyle) {
    mJoinStyle = joinStyle;
}

void StrokeSettings::setStrokerSettings(QPainterPathStroker *stroker) {
    stroker->setWidth(mLineWidth.getCurrentValue());
    stroker->setCapStyle(mCapStyle);
    stroker->setJoinStyle(mJoinStyle);
}

bool StrokeSettings::nonZeroLineWidth() {
    return !isZero(mLineWidth.getCurrentValue());
}

#include "qdoubleslider.h"
FillStrokeSettingsWidget::FillStrokeSettingsWidget(MainWindow *parent) : QWidget(parent)
{
    //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mMainWindow = parent;
    mUndoRedoSaveTimer = new QTimer(this);

    mGradientWidget = new GradientWidget(this, mMainWindow);
    mColorTypeBar = new QTabBar(this);
    mColorTypeBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
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
    mFillTargetButton->setFocusPolicy(Qt::NoFocus);
    mStrokeTargetButton->setCheckable(true);
    mStrokeTargetButton->setFocusPolicy(Qt::NoFocus);
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

    //mLineWidthSpin = new QDoubleSpinBox(this);
    mLineWidthSpin = new QDoubleSlider("line width", 0., 1000., 1., this);
    mLineWidthSpin->setNameVisible(false);
    //mLineWidthSpin->setValueSliderVisibile(false);
    //mLineWidthSpin->setRange(0.0, 1000.0);
    //mLineWidthSpin->setSuffix(" px");
    //mLineWidthSpin->setSingleStep(0.1);
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
    connect(mColorTypeBar, SIGNAL(tabBarClicked(int)),
            this, SLOT(colorTypeSet(int)) );

    connect(mColorsSettingsWidget,
                SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
                this, SLOT(colorChangedTMP(GLfloat,GLfloat,GLfloat,GLfloat)) );

    mFillPickerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_fill_dropper.png"), "", this);
    mFillPickerButton->setSizePolicy(QSizePolicy::Maximum,
                                     QSizePolicy::Maximum);
    mFillPickerButton->setFocusPolicy(Qt::NoFocus);
    mStrokePickerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_stroke_dropper.png"), "", this);
    mStrokePickerButton->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    mStrokePickerButton->setFocusPolicy(Qt::NoFocus);
    mFillStrokePickerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_fill_stroke_dropper.png"), "", this);
    mFillStrokePickerButton->setSizePolicy(QSizePolicy::Maximum,
                                           QSizePolicy::Maximum);
    mFillStrokePickerButton->setFocusPolicy(Qt::NoFocus);
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

void FillStrokeSettingsWidget::saveGradientsToSqlIfPathSelected(QSqlQuery *query) {
    mGradientWidget->saveGradientsToSqlIfPathSelected(query);
}

void FillStrokeSettingsWidget::updateAfterTargetChanged() {
    if(getCurrentPaintTypeVal() == GRADIENTPAINT) {
        mGradientWidget->setCurrentGradient(getCurrentGradientVal() );
    }
    setCurrentPaintType(getCurrentPaintTypeVal());
    mColorTypeBar->setCurrentIndex(getCurrentPaintTypeVal());
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

void FillStrokeSettingsWidget::setTransformFinishEmitter(const char *slot) {
    mUndoRedoSaveTimer->disconnect();
    connect(mUndoRedoSaveTimer, SIGNAL(timeout()),
            this, SLOT(finishTransform() ) );
    connect(mUndoRedoSaveTimer, SIGNAL(timeout()),
            this, slot );
}

void FillStrokeSettingsWidget::setStrokeWidth(qreal width)
{
    mCanvas->startSelectedStrokeWidthTransform();
    startTransform(SLOT(emitStrokeWidthChanged()));
    mCurrentStrokeWidth = width;
    emitStrokeWidthChangedTMP();
}

void FillStrokeSettingsWidget::setCurrentSettings(const PaintSettings *fillPaintSettings,
                                            const StrokeSettings *strokePaintSettings)
{
    if(fillPaintSettings == NULL || strokePaintSettings == NULL) return;
    disconnect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));

    setFillValuesFromFillSettings(fillPaintSettings);
    setStrokeValuesFromStrokeSettings(strokePaintSettings);
    mLineWidthSpin->setValue(strokePaintSettings->getCurrentStrokeWidth());

    if(mTargetId == 0) {
        setFillTarget();
    } else {
        setStrokeTarget();
    }

    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
}

void FillStrokeSettingsWidget::setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a) {
    mColorsSettingsWidget->setCurrentColor(h, s, v, a);
}

void FillStrokeSettingsWidget::setCurrentColor(Color color) {
    mColorsSettingsWidget->setCurrentColor(color);
}

void FillStrokeSettingsWidget::saveGradientsToQuery(QSqlQuery *query)
{
    mGradientWidget->saveGradientsToQuery(query);
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
        if((mTargetId == 0) ? (mCurrentFillGradient == NULL) :
                (mCurrentStrokeGradient == NULL) ) {
            mGradientWidget->setCurrentGradient((Gradient*)NULL);
        }
        setGradientPaintType();
    }
    emitPaintTypeChanged();
}

void FillStrokeSettingsWidget::colorChangedTMP(GLfloat h, GLfloat s, GLfloat v,
                                               GLfloat a)
{
    if(mTargetId == 0) {
        if(mCurrentFillPaintType == GRADIENTPAINT) {
            mGradientWidget->startSelectedColorTransform();
        } else if(mCurrentFillPaintType == FLATPAINT) {
            mCanvas->startSelectedFillColorTransform();
        }
    } else {
        if(mCurrentStrokePaintType == GRADIENTPAINT) {
            mGradientWidget->startSelectedColorTransform();
        } else if(mCurrentStrokePaintType == FLATPAINT) {
            mCanvas->startSelectedStrokeColorTransform();
        }
    }
    startTransform(SLOT(emitColorSettingsChanged()));
    if(getCurrentPaintTypeVal() == FLATPAINT) {
        flatColorSet(h, s, v, a);
        emitFlatColorChangedTMP();
    } else if(getCurrentPaintTypeVal() == GRADIENTPAINT) {
        mGradientWidget->setCurrentColor(h, s, v, a);
        emitGradientChangedTMP();
    }
}

void FillStrokeSettingsWidget::flatColorSet(GLfloat h, GLfloat s, GLfloat v,
                                            GLfloat a)
{
    if(mTargetId == 0) {
        mCurrentFillColor.setHSV(h, s, v, a);
    } else {
        mCurrentStrokeColor.setHSV(h, s, v, a);
    }
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
    mCurrentJoinStyle = joinStyle;
    mBevelJoinStyleButton->setChecked(joinStyle == Qt::BevelJoin);
    mMiterJointStyleButton->setChecked(joinStyle == Qt::MiterJoin);
    mRoundJoinStyleButton->setChecked(joinStyle == Qt::RoundJoin);
}

void FillStrokeSettingsWidget::setCapStyle(Qt::PenCapStyle capStyle)
{
    mCurrentCapStyle = capStyle;
    mFlatCapStyleButton->setChecked(capStyle == Qt::FlatCap);
    mSquareCapStyleButton->setChecked(capStyle == Qt::SquareCap);
    mRoundCapStyleButton->setChecked(capStyle == Qt::RoundCap);
}

void FillStrokeSettingsWidget::setFillValuesFromFillSettings(
        const PaintSettings *settings) {
    mCurrentFillColor = settings->getCurrentColor();
    mCurrentFillGradient = settings->getGradient();
    mCurrentFillPaintType = settings->getPaintType();
}

void FillStrokeSettingsWidget::setStrokeValuesFromStrokeSettings(
        const StrokeSettings *settings) {
    mCurrentStrokeColor = settings->getCurrentColor();
    mCurrentStrokeGradient = settings->getGradient();
    mCurrentStrokePaintType = settings->getPaintType();
    mCurrentStrokeWidth = settings->getCurrentStrokeWidth();
    mCurrentCapStyle = settings->getCapStyle();
    mCurrentJoinStyle = settings->getJoinStyle();
}

void FillStrokeSettingsWidget::setCanvasPtr(Canvas *canvas)
{
    mCanvas = canvas;
}

void FillStrokeSettingsWidget::loadSettingsFromPath(PathBox *path) {
    if(mLoadFillFromPath) {
        mLoadFillFromPath = false;
        setFillValuesFromFillSettings(path->getFillSettings());
        mCanvas->fillPaintTypeChanged(mCurrentFillPaintType,
                                    mCurrentFillColor,
                                    mCurrentFillGradient);
    }
    if(mLoadStrokeFromPath) {
        mLoadStrokeFromPath = false;
        setStrokeValuesFromStrokeSettings(path->getStrokeSettings());

        mCanvas->strokePaintTypeChanged(mCurrentStrokePaintType,
                                        mCurrentStrokeColor,
                                        mCurrentStrokeGradient);
        mCanvas->strokeCapStyleChanged(mCurrentCapStyle);
        mCanvas->strokeJoinStyleChanged(mCurrentJoinStyle);
        mCanvas->strokeWidthChanged(mCurrentStrokeWidth, true);
    }
}

void FillStrokeSettingsWidget::emitStrokeFlatColorChanged() {
    mCanvas->strokeFlatColorChanged(mCurrentStrokeColor, true);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitStrokeFlatColorChangedTMP() {
    mCanvas->strokeFlatColorChanged(mCurrentStrokeColor, false);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitStrokeGradientChanged() {
    mCanvas->strokeGradientChanged(mCurrentStrokeGradient, true);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitStrokeGradientChangedTMP() {
    mCanvas->strokeGradientChanged(mCurrentStrokeGradient, false);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitStrokePaintTypeChanged() {
    mCanvas->strokePaintTypeChanged(mCurrentStrokePaintType,
                                    mCurrentStrokeColor,
                                    mCurrentStrokeGradient);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitFillFlatColorChanged() {
    mCanvas->fillFlatColorChanged(mCurrentFillColor, true);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitFillFlatColorChangedTMP() {
    mCanvas->fillFlatColorChanged(mCurrentFillColor, false);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitFillGradientChanged() {
    mCanvas->fillGradientChanged(mCurrentFillGradient, true);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitFillGradientChangedTMP() {
    mCanvas->fillGradientChanged(mCurrentFillGradient, false);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitFillPaintTypeChanged() {
    mCanvas->fillPaintTypeChanged(mCurrentFillPaintType,
                                mCurrentFillColor,
                                mCurrentFillGradient);

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitFlatColorChanged() {
    if(mTargetId == 0) {
        emitFillFlatColorChanged();
    } else {
        emitStrokeFlatColorChanged();
    }
}

void FillStrokeSettingsWidget::emitFlatColorChangedTMP() {
    if(mTargetId == 0) {
        emitFillFlatColorChangedTMP();
    } else {
        emitStrokeFlatColorChangedTMP();
    }
}

void FillStrokeSettingsWidget::emitGradientChanged() {
    if(mTargetId == 0) {
        emitFillGradientChanged();
    } else {
        emitStrokeGradientChanged();
    }
}

void FillStrokeSettingsWidget::emitGradientChangedTMP() {
    if(mTargetId == 0) {
        emitFillGradientChangedTMP();
    } else {
        emitStrokeGradientChangedTMP();
    }
}

void FillStrokeSettingsWidget::emitColorSettingsChangedTMP() {
    if(getCurrentPaintTypeVal() == PaintType::GRADIENTPAINT) {
        emitGradientChangedTMP();
    } else {
        emitFlatColorChangedTMP();
    }
}


void FillStrokeSettingsWidget::emitColorSettingsChanged() {
    if(getCurrentPaintTypeVal() == PaintType::GRADIENTPAINT) {
        mGradientWidget->finishGradientTransform();
        emitGradientChanged();
    } else {
        emitFlatColorChanged();
    }
}

void FillStrokeSettingsWidget::emitPaintTypeChanged() {
    if(mTargetId == 0) {
        emitFillPaintTypeChanged();
    } else {
        emitStrokePaintTypeChanged();
    }
}

void FillStrokeSettingsWidget::emitStrokeWidthChanged() {
    mCanvas->strokeWidthChanged(mCurrentStrokeWidth, true);
    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitStrokeWidthChangedTMP() {
    mCanvas->strokeWidthChanged(mCurrentStrokeWidth, false);
    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitCapStyleChanged() {
    mCanvas->strokeCapStyleChanged(mCurrentCapStyle);
    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::emitJoinStyleChanged() {
    mCanvas->strokeJoinStyleChanged(mCurrentJoinStyle);
    mMainWindow->callUpdateSchedulers();
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
    }
}

void FillStrokeSettingsWidget::startTransform(const char *slot)
{
    if(!mTransormStarted) {
        mTransormStarted = true;
        setTransformFinishEmitter(slot);
    }
    waitToSaveChanges();
    /*if(mCurrentPaintType == GRADIENTPAINT) {
        mGradientWidget->startGradientTransform();
    } else {
        if(mTargetId == 0) {
            emit startFillSettingsTransform();
        } else {
            emit startStrokeSettingsTransform();
        }
    }*/
}

void FillStrokeSettingsWidget::setGradient(Gradient *gradient)
{
    setCurrentGradientVal(gradient);
    emitGradientChanged();
}

void FillStrokeSettingsWidget::setBevelJoinStyle()
{
    setJoinStyle(Qt::BevelJoin);
    emitJoinStyleChanged();
}

void FillStrokeSettingsWidget::setMiterJoinStyle()
{
    setJoinStyle(Qt::MiterJoin);
    emitJoinStyleChanged();
}

void FillStrokeSettingsWidget::setRoundJoinStyle()
{
    setJoinStyle(Qt::RoundJoin);
    emitJoinStyleChanged();
}

void FillStrokeSettingsWidget::setFlatCapStyle()
{
    setCapStyle(Qt::FlatCap);
    emitCapStyleChanged();
}

void FillStrokeSettingsWidget::setSquareCapStyle()
{
    setCapStyle(Qt::SquareCap);
    emitCapStyleChanged();
}

void FillStrokeSettingsWidget::setRoundCapStyle()
{
    setCapStyle(Qt::RoundCap);
    emitCapStyleChanged();
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
    updateAfterTargetChanged();
}

void FillStrokeSettingsWidget::setStrokeTarget()
{
    mTargetId = 1;
    mStrokeTargetButton->setChecked(true);
    mFillTargetButton->setChecked(false);
    mStrokeSettingsWidget->show();
    updateAfterTargetChanged();
}

void FillStrokeSettingsWidget::setNoPaintType()
{
    setCurrentPaintTypeVal(NOPAINT);
    mColorsSettingsWidget->hide();
    mGradientWidget->hide();
}

void FillStrokeSettingsWidget::setFlatPaintType()
{
    disconnectGradient();
    mColorsSettingsWidget->show();
    mGradientWidget->hide();
    mColorsSettingsWidget->setCurrentColor(getCurrentColorVal());
    setCurrentPaintTypeVal(FLATPAINT);
}

void FillStrokeSettingsWidget::setGradientPaintType()
{
    connectGradient();
    if(mTargetId == 0) {
        mCurrentFillPaintType = GRADIENTPAINT;
        if(mCurrentFillGradient == NULL) {
            mCurrentFillGradient = mGradientWidget->getCurrentGradient();
        }
    } else {
        mCurrentStrokePaintType = GRADIENTPAINT;
        if(mCurrentStrokeGradient == NULL) {
            mCurrentStrokeGradient = mGradientWidget->getCurrentGradient();
        }
    }
    mColorsSettingsWidget->show();
    mGradientWidget->show();
    mColorsSettingsWidget->setCurrentColor(mGradientWidget->getCurrentColor());

    mGradientWidget->update();
}
