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

#include "exportsvgdialog.h"

#include "Private/document.h"
#include "GUI/scenechooser.h"
#include "GUI/global.h"
#include "canvas.h"
#include "svgexporter.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDomDocument>
#include <QMenuBar>

ExportSvgDialog::ExportSvgDialog(const QString& path,
                                 QWidget* const parent) :
    QDialog(parent), mPath(path) {

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setWindowTitle("Export SVG");

    const auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    const auto layout = new QHBoxLayout();
    mainLayout->addLayout(layout);
    const auto labelLayout = new QVBoxLayout();
    const auto spinLayout = new QVBoxLayout();
    layout->addLayout(labelLayout);
    layout->addLayout(spinLayout);

    const auto document = Document::sInstance;
    mScene = new SceneChooser(*document, false, this);
    auto scene = *document->fActiveScene;
    if(!scene) {
        const auto& visScenes = document->fVisibleScenes;
        if(!visScenes.empty()) scene = visScenes.begin()->first;
        else {
            const auto& scenes = document->fScenes;
            if(!scenes.isEmpty()) scene = scenes.first().get();
        }
    }
    mScene->setCurrentScene(scene);

    const auto sceneButton = new QPushButton(mScene->title(), this);
    sceneButton->setMenu(mScene);

    mFirstFrame = new QSpinBox(this);
    mLastFrame = new QSpinBox(this);

    const int minFrame = scene ? scene->getMinFrame() : 0;
    const int maxFrame = scene ? scene->getMaxFrame() : 0;

    mFirstFrame->setRange(-99999, maxFrame);
    mFirstFrame->setValue(minFrame);
    mLastFrame->setRange(minFrame, 99999);
    mLastFrame->setValue(maxFrame);

    mBackground = new QCheckBox("Background", this);
    mBackground->setChecked(true);
    mFixedSize = new QCheckBox("Fixed Size", this);
    mFixedSize->setChecked(false);
    mLoop = new QCheckBox("Loop", this);
    mLoop->setChecked(true);

    spinLayout->addWidget(sceneButton);
    spinLayout->addSpacing(MIN_WIDGET_DIM);
    spinLayout->addWidget(mFirstFrame);
    spinLayout->addWidget(mLastFrame);

    mainLayout->addSpacing(MIN_WIDGET_DIM);
    mainLayout->addWidget(mBackground);
    mainLayout->addWidget(mFixedSize);
    mainLayout->addWidget(mLoop);
    mainLayout->addSpacing(MIN_WIDGET_DIM);

    connect(mFirstFrame, qOverload<int>(&QSpinBox::valueChanged),
            mLastFrame, &QSpinBox::setMinimum);
    connect(mLastFrame, qOverload<int>(&QSpinBox::valueChanged),
            mFirstFrame, &QSpinBox::setMaximum);

    labelLayout->addWidget(new QLabel("Scene:"));
    labelLayout->addSpacing(MIN_WIDGET_DIM);
    labelLayout->addWidget(new QLabel("First Frame:"));
    labelLayout->addWidget(new QLabel("Last Frame:"));

    const auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
                                              QDialogButtonBox::Cancel);

    connect(mScene, &SceneChooser::currentChanged,
            this, [this, buttons, sceneButton](Canvas* const scene) {
        buttons->button(QDialogButtonBox::Ok)->setEnabled(scene);
        sceneButton->setText(mScene->title());
    });

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        try {
            const auto scene = mScene->getCurrentScene();
            if(!scene) RuntimeThrow("No scene selected");
            const int firstFrame = mFirstFrame->value();
            const int lastFrame = mLastFrame->value();

            const bool background = mBackground->isChecked();
            const bool fixedSize = mFixedSize->isChecked();
            const bool loop = mLoop->isChecked();

            const FrameRange frameRange{firstFrame, lastFrame};
            const qreal fps = scene->getFps();

            const auto task = new SvgExporter(mPath, scene, frameRange, fps,
                                              background, fixedSize, loop);
            const auto taskSPtr = QSharedPointer<SvgExporter>(task);
            task->nextStep();
            TaskScheduler::instance()->addComplexTask(taskSPtr);
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
            return;
        }

        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttons);
}
