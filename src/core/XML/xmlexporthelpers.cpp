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

#include "xmlexporthelpers.h"

#include "exceptions.h"

SkBlendMode XmlExportHelpers::stringToBlendMode(const QString& compOpStr) {
    if(compOpStr.isEmpty()) return SkBlendMode::kSrcOver;
    if(compOpStr == "svg:src-over") {
        return SkBlendMode::kSrcOver;
    } else if(compOpStr == "svg:multiply") {
        return SkBlendMode::kMultiply;
    } else if(compOpStr == "svg:screen") {
        return SkBlendMode::kScreen;
    } else if(compOpStr == "svg:overlay") {
        return SkBlendMode::kOverlay;
    } else if(compOpStr == "svg:darken") {
        return SkBlendMode::kDarken;
    } else if(compOpStr == "svg:lighten") {
        return SkBlendMode::kLighten;
    } else if(compOpStr == "svg:color-dodge") {
        return SkBlendMode::kColorDodge;
    } else if(compOpStr == "svg:color-burn") {
        return SkBlendMode::kColorBurn;
    } else if(compOpStr == "svg:hard-light") {
        return SkBlendMode::kHardLight;
    } else if(compOpStr == "svg:soft-light") {
        return SkBlendMode::kSoftLight;
    } else if(compOpStr == "svg:difference") {
        return SkBlendMode::kDifference;
    } else if(compOpStr == "svg:color") {
        return SkBlendMode::kColor;
    } else if(compOpStr == "svg:luminosity") {
        return SkBlendMode::kLuminosity;
    } else if(compOpStr == "svg:hue") {
        return SkBlendMode::kHue;
    } else if(compOpStr == "svg:saturation") {
        return SkBlendMode::kSaturation;
    } else if(compOpStr == "svg:plus") {
        return SkBlendMode::kPlus;
    } else if(compOpStr == "svg:dst-in") {
        return SkBlendMode::kDstIn;
    } else if(compOpStr == "svg:dst-out") {
        return SkBlendMode::kDstOut;
    } else if(compOpStr == "svg:src-atop") {
        return SkBlendMode::kSrcATop;
    } else if(compOpStr == "svg:dst-atop") {
        return SkBlendMode::kDstATop;
    } else return SkBlendMode::kSrcOver;
}

QString XmlExportHelpers::blendModeToString(const SkBlendMode blendMode) {
    switch(blendMode) {
    case SkBlendMode::kSrcOver: return "svg:src-over";
    case SkBlendMode::kMultiply: return "svg:multiply";
    case SkBlendMode::kScreen: return "svg:screen";
    case SkBlendMode::kOverlay: return "svg:overlay";
    case SkBlendMode::kDarken: return "svg:darken";
    case SkBlendMode::kLighten: return "svg:lighten";
    case SkBlendMode::kColorDodge: return "svg:color-dodge";
    case SkBlendMode::kColorBurn: return "svg:color-burn";
    case SkBlendMode::kHardLight: return "svg:hard-light";
    case SkBlendMode::kSoftLight: return "svg:soft-light";
    case SkBlendMode::kDifference: return "svg:difference";
    case SkBlendMode::kColor: return "svg:color";
    case SkBlendMode::kLuminosity: return "svg:luminosity";
    case SkBlendMode::kHue: return "svg:hue";
    case SkBlendMode::kSaturation: return "svg:saturation";
    case SkBlendMode::kPlus: return "svg:plus";
    case SkBlendMode::kDstIn: return "svg:dst-in";
    case SkBlendMode::kDstOut: return "svg:dst-out";
    case SkBlendMode::kSrcATop: return "svg:src-atop";
    case SkBlendMode::kDstATop: return "svg:dst-atop";
    default: return "svg:src-over";
    }
}

qreal XmlExportHelpers::stringToDouble(const QStringRef& string) {
    bool ok;
    const qreal value = string.toDouble(&ok);
    if(!ok) RuntimeThrow("Invalid value " + string.toString());
    return value;
}

qreal XmlExportHelpers::stringToDouble(const QString& string) {
    return stringToDouble(&string);
}

int XmlExportHelpers::stringToInt(const QStringRef& string) {
    bool ok;
    const int value = string.toInt(&ok);
    if(!ok) RuntimeThrow("Invalid value " + string.toString());
    return value;
}

int XmlExportHelpers::stringToInt(const QString& string) {
    return stringToInt(&string);
}

QDomElement XmlExportHelpers::getOnlyElement(
        const QDomNode& from, const QString& tagName) {
    const auto eles = from.childNodes();
    const int count = eles.count();
    for(int i = 0; i < count; i++) {
        const auto node = eles.at(i);
        if(!node.isElement()) continue;
        const auto ele = node.toElement();
        if(ele.tagName() == tagName) return ele;
    }
    return QDomElement();
}
