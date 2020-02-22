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

#include "eindependentsound.h"

#include "Timeline/fixedlenanimationrect.h"
#include "fileshandler.h"

SoundFileHandler* soundFileHandlerGetter(const QString& path) {
    return FilesHandler::sInstance->getFileHandler<SoundFileHandler>(path);
}

qsptr<FixedLenAnimationRect> createIndependentSoundDur(
        eIndependentSound* const sound) {
    const auto result = enve::make_shared<FixedLenAnimationRect>(*sound);
    result->setBindToAnimationFrameRange();
    return result;
}

eIndependentSound::eIndependentSound() :
    eSoundObjectBase(createIndependentSoundDur(this)),
    mFileHandler(this,
                 [](const QString& path) {
                     return soundFileHandlerGetter(path);
                 },
                 [this](SoundFileHandler* obj) {
                     fileHandlerAfterAssigned(obj);
                 },
                 [this](ConnContext& conn, SoundFileHandler* obj) {
                     fileHandlerConnector(conn, obj);
                 }) {

}

void eIndependentSound::fileHandlerConnector(ConnContext &conn, SoundFileHandler *obj) {
    conn << connect(obj, &SoundFileHandler::pathChanged,
                    this, &eSoundObjectBase::prp_afterWholeInfluenceRangeChanged);
    conn << connect(obj, &SoundFileHandler::reloaded,
                    this, &eSoundObjectBase::prp_afterWholeInfluenceRangeChanged);
}

void eIndependentSound::fileHandlerAfterAssigned(SoundFileHandler *obj) {
    if(obj) {
        const auto newDataHandler = FileDataCacheHandler::
                sGetDataHandler<SoundDataHandler>(obj->path());
        setSoundDataHandler(newDataHandler);
        prp_setName(QFileInfo(obj->path()).fileName());
    } else {
        setSoundDataHandler(nullptr);
        prp_setName("Sound");
    }
}

#include <QInputDialog>
#include "typemenu.h"
void eIndependentSound::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<eIndependentSound>()) return;
    menu->addedActionsForType<eIndependentSound>();
    eSoundObjectBase::prp_setupTreeViewMenu(menu);
    const auto widget = menu->getParentWidget();
    const PropertyMenu::PlainSelectedOp<eIndependentSound> stretchOp =
            [this, widget](eIndependentSound * sound) {
        bool ok = false;
        const int stretch = QInputDialog::getInt(
                    widget, "Stretch " + sound->prp_getName(),
                    "Stretch:", qRound(getStretch()*100),
                    -1000, 1000, 1, &ok);
        if(!ok) return;
        sound->setStretch(stretch*0.01);
    };
    menu->addPlainAction("Stretch...", stretchOp);

    const PropertyMenu::PlainTriggeredOp deleteOp = [this]() {
        removeFromParent_k();
    };
    menu->addPlainAction("Delete", deleteOp);
}

bool eIndependentSound::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                            const bool parentSatisfies,
                                            const bool parentMainTarget) const {
    Q_UNUSED(parentMainTarget);
    if(rules.fRule == SWT_BoxRule::visible && !isVisible()) return false;
    if(rules.fRule == SWT_BoxRule::selected && !isSelected()) return false;
    if(rules.fType == SWT_Type::sound) return true;
    if(rules.fType == SWT_Type::graphics) return false;
    return parentSatisfies;
}

void eIndependentSound::setFilePath(const QString &path) {
    mFileHandler.assign(path);
}

void eIndependentSound::updateDurationRectLength() {
    if(cacheHandler() && getParentScene()) {
        const qreal secs = durationSeconds();
        const qreal fps = getCanvasFPS();
        const int frames = qCeil(qAbs(secs*fps*getStretch()));
        const auto flaRect = static_cast<FixedLenAnimationRect*>(
                    getDurationRectangle());
        flaRect->setAnimationFrameDuration(frames);
    }
}

#include "ReadWrite/basicreadwrite.h"
void eIndependentSound::prp_writeProperty(eWriteStream& dst) const {
    eBoxOrSound::prp_writeProperty(dst);
    const auto cacheHandler = this->cacheHandler();
    const auto filePath = cacheHandler ? cacheHandler->getFilePath() : "";
    dst << filePath;
}

void eIndependentSound::prp_readProperty(eReadStream& src) {
    eBoxOrSound::prp_readProperty(src);
    QString filePath;
    src >> filePath;
    if(!filePath.isEmpty()) setFilePath(filePath);
}
