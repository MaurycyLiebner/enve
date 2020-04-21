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
