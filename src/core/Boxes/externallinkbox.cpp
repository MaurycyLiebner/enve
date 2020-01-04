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

#include "externallinkbox.h"

#include "GUI/edialogs.h"

ExternalLinkBox::ExternalLinkBox() : ContainerBox(eBoxType::layer) {
    mType = eBoxType::externalLink;
    prp_setName("Link Empty");
}

void ExternalLinkBox::reload() {


    planUpdate(UpdateReason::userChange);
}

void ExternalLinkBox::changeSrc() {
    QString src = eDialogs::openFile("Link File", mSrc,
                                     "enve Files (*.ev)");
    if(!src.isEmpty()) setSrc(src);
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    prp_setName("Link " + src);
    reload();
}
