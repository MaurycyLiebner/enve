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

#include "svgexporter.h"

#include "canvas.h"

SvgExporter::SvgExporter(const QString& path, Canvas* const scene,
                         const FrameRange& frameRange,
                         const qreal fps, const bool background,
                         const bool fixedSize, const bool loop) :
    ComplexTask(INT_MAX, "SVG Export"),
    fScene(scene), fAbsRange(frameRange), fFps(fps),
    fBackground(background), fFixedSize(fixedSize), fLoop(loop),
    mFile(path), mSvg(createElement("svg")),
    mDefs(createElement("defs")) {}

void SvgExporter::nextStep() {
    if(!mOpen) {
        if(mFile.open(QIODevice::WriteOnly)) {
            mStream.setDevice(&mFile);
            mStream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
            mStream << "<!-- Created with enve https://maurycyliebner.github.io -->" << endl << endl;
            fScene->saveSceneSVG(*this);
        } else {
            RuntimeThrow("Could not open:\n\"" + mFile.fileName() + "\"");
        }
        mOpen = true;
    }
    if(mWaitingTasks.isEmpty()) return finish();
    const auto task = mWaitingTasks.takeFirst();
    addTask(task);
}

void SvgExporter::addNextTask(const stdsptr<eTask>& task) {
    mWaitingTasks << task;
}

void SvgExporter::finish() {
    if(mOpen) {
        mSvg.appendChild(mDefs);
        mDoc.appendChild(mSvg);
        mStream << mDoc.toString();
        mStream.flush();
        mFile.close();
    }
    setValue(INT_MAX);
}
