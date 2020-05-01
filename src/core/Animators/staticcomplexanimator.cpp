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

#include "staticcomplexanimator.h"

StaticComplexAnimator::StaticComplexAnimator(const QString &name) :
    ComplexAnimator(name) {}

void StaticComplexAnimator::prp_writeProperty(eWriteStream &dst) const {
    const auto& children = ca_getChildren();
    for(const auto& prop : children)
        prop->prp_writeProperty(dst);
}

void StaticComplexAnimator::prp_readProperty(eReadStream &src) {
    const auto& children = ca_getChildren();
    for(const auto& prop : children)
        prop->prp_readProperty(src);
}

void StaticComplexAnimator::prp_readPropertyXEV_impl(const QDomElement& ele, const XevImporter& imp) {
    const auto& children = ca_getChildren();
    const auto childNodes = ele.childNodes();
    const int count = childNodes.count();
    for(int i = 0; i < count; i++) {
        const auto node = childNodes.at(i);
        if(!node.isElement()) continue;
        const auto element = node.toElement();
        const auto& prop = children.at(i);
        const auto path = QString::number(i) + "/";
        prop->prp_readPropertyXEV(element, imp.withAssetsPath(path));
    }
}

void StaticComplexAnimator::writeChildPropertiesXEV(
        QDomElement& prop, const XevExporter& exp) const {
    const auto& children = ca_getChildren();
    int id = 0;
    for(const auto& c : children) {
        const auto path = QString::number(id++) + "/";
        const auto expc = exp.withAssetsPath(path);
        auto child = c->prp_writePropertyXEV(*expc);
        prop.appendChild(child);
    }
}

QDomElement StaticComplexAnimator::prp_writePropertyXEV_impl(const XevExporter& exp) const {
    auto result = exp.createElement(prp_tagNameXEV());
    writeChildPropertiesXEV(result, exp);
    return result;
}
