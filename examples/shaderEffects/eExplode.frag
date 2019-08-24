// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
uniform vec2 displacementXY;
uniform int steps;
uniform int directions;

const float PIx2 = 6.28318530718;

void main(void) {
    vec4 Color = texture2D(texture, texCoord);
    for(float d = 0.0; d < PIx2; d += PIx2/float(directions)) {
        for(float i= 1.0/float(steps); i <= 1.0; i += 1.0/float(steps)) {
            Color += texture2D(texture, texCoord + vec2(cos(d), sin(d))*displacementXY*i);
        }
    }
    Color /= float(steps)*float(directions) + 1.0;
    gl_FragColor =  Color;
}
