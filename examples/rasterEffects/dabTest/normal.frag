#version 330 core
in vec2 texCoord;

uniform sampler2D dabData;

void main(void) {
    float fX = texelFetch(dabData, ivec2(0, 0), 0).r; // 0
    float fY = texelFetch(dabData, ivec2(1, 0), 0).r; // 1
    float fRadius = texelFetch(dabData, ivec2(2, 0), 0).r; // 2
    float fAspectRatio = texelFetch(dabData, ivec2(3, 0), 0).r; // 3
    float fSinTh = texelFetch(dabData, ivec2(4, 0), 0).r; // 4
    float fCosTh = texelFetch(dabData, ivec2(5, 0), 0).r; // 5
    float fHardness = texelFetch(dabData, ivec2(6, 0), 0).r; // 6
    float fOpaque = texelFetch(dabData, ivec2(7, 0), 0).r; // 7
    float fRed = texelFetch(dabData, ivec2(8, 0), 0).r; // 8
    float fGreen = texelFetch(dabData, ivec2(9, 0), 0).r; // 9
    float fBlue = texelFetch(dabData, ivec2(10, 0), 0).r; // 10
    float fAlpha = texelFetch(dabData, ivec2(11, 0), 0).r; // 11
    float fSeg1Offset = texelFetch(dabData, ivec2(12, 0), 0).r; // 12
    float fSeg1Slope = texelFetch(dabData, ivec2(13, 0), 0).r; // 13
    float fSeg2Offset = texelFetch(dabData, ivec2(14, 0), 0).r; // 14
    float fSeg2Slope = texelFetch(dabData, ivec2(15, 0), 0).r; // 15

	float xp = texCoord.x;
	float yp = texCoord.y;

    float yy = (yp - fY);
    float xx = (xp - fX);
    float yyr = (yy*fCosTh - xx*fSinTh) * fAspectRatio;
    float xxr = yy*fSinTh + xx*fCosTh;
    float rr = (yyr*yyr + xxr*xxr)/(fRadius*fRadius);
	
	if(rr > 1.f) {
		gl_FragColor = vec4(0.f, 0.f, 0.f, 0.f);
	} else {
		float fac = (rr <= fHardness ? fSeg1Slope : fSeg2Slope);
		float opa = (rr <= fHardness ? fSeg1Offset : fSeg2Offset) + rr*fac;
		opa *= fAlpha;
    	gl_FragColor = vec4(fRed*opa, fGreen*opa, fBlue*opa, opa);
	}
}
