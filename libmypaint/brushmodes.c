/* libmypaint - The MyPaint Brush Library
 * Copyright (C) 2007-2014 Martin Renold <martinxyz@gmx.ch> et. al
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>

#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "fastapprox/fastpow.h"

#include "helpers.h"

// parameters to those methods:
//
// rgba: A pointer to 16bit rgba data with premultiplied alpha.
//       The range of each components is limited from 0 to 2^15.
//
// mask: Contains the dab shape, that is, the intensity of the dab at
//       each pixel. Usually rendering is done for one tile at a
//       time. The mask is LRE encoded to jump quickly over regions
//       that are not affected by the dab.
//
// opacity: overall strength of the blending mode. Has the same
//          influence on the dab as the values inside the mask.


// We are manipulating pixels with premultiplied alpha directly.
// This is an "over" operation (opa = topAlpha).
// In the formula below, topColor is assumed to be premultiplied.
//
//               opa_a      <   opa_b      >
// resultAlpha = topAlpha + (1.0 - topAlpha) * bottomAlpha
// resultColor = topColor + (1.0 - topAlpha) * bottomColor
//

void draw_dab_pixels_BlendMode_Normal (uint16_t * mask,
                                       uint16_t * rgba,
                                       uint16_t color_r,
                                       uint16_t color_g,
                                       uint16_t color_b,
                                       uint16_t opacity) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);
      rgba[0] = (opa_a*color_r + opa_b*rgba[0])/(1<<15);
      rgba[1] = (opa_a*color_g + opa_b*rgba[1])/(1<<15);
      rgba[2] = (opa_a*color_b + opa_b*rgba[2])/(1<<15);

    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

void draw_dab_pixels_BlendMode_Normal_Paint (uint16_t * mask,
                                       uint16_t * rgba,
                                       uint16_t color_r,
                                       uint16_t color_g,
                                       uint16_t color_b,
                                       uint16_t opacity) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      // pigment-mode does not like very low opacity, probably due to rounding
      // errors with int->float->int round-trip.  Once we convert to pure
      // float engine this might be fixed.  For now enforce a minimum opacity:
      opacity = MAX(opacity, 150);
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      // optimization- if background has 0 alpha we can just do normal additive
      // blending since there is nothing to mix with.
      if (rgba[3] <= 0) {
        rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);
        rgba[0] = (opa_a*color_r + opa_b*rgba[0])/(1<<15);
        rgba[1] = (opa_a*color_g + opa_b*rgba[1])/(1<<15);
        rgba[2] = (opa_a*color_b + opa_b*rgba[2])/(1<<15);
        continue;
      }
      //alpha-weighted ratio for WGM (sums to 1.0)
      float fac_a = (float)opa_a / (opa_a + opa_b * rgba[3] / (1<<15));
      float fac_b = 1.0 - fac_a;

      //convert bottom to spectral.  Un-premult alpha to obtain reflectance
      //color noise is not a problem since low alpha also implies low weight
      float spectral_b[10] = {0};

      rgb_to_spectral((float)rgba[0] / rgba[3], (float)rgba[1] / rgba[3], (float)rgba[2] / rgba[3], spectral_b);

      // convert top to spectral.  Already straight color
      float spectral_a[10] = {0};
      rgb_to_spectral((float)color_r / (1<<15), (float)color_g / (1<<15), (float)color_b / (1<<15), spectral_a);

      // mix to the two spectral reflectances using WGM
      float spectral_result[10] = {0};
      for (int i=0; i<10; i++) {
        spectral_result[i] = fastpow(spectral_a[i], fac_a) * fastpow(spectral_b[i], fac_b);
      }
      
      // convert back to RGB and premultiply alpha
      float rgb_result[3] = {0};
      spectral_to_rgb(spectral_result, rgb_result);
      rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);

      for (int i=0; i<3; i++) {
        rgba[i] =(rgb_result[i] * rgba[3]) + 0.5;
      }
    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

//Posterize.  Basically exactly like GIMP's posterize
//reduces colors by adjustable amount (posterize_num).
//posterize the canvas, then blend that via opacity
//does not affect alpha

void draw_dab_pixels_BlendMode_Posterize (uint16_t * mask,
                                       uint16_t * rgba,
                                       uint16_t opacity,
                                       uint16_t posterize_num) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
     
      float r = (float)rgba[0] / (1<<15);
      float g = (float)rgba[1] / (1<<15);
      float b = (float)rgba[2] / (1<<15);

      uint32_t post_r = (1<<15) * ROUND(r * posterize_num) / posterize_num;
      uint32_t post_g = (1<<15) * ROUND(g * posterize_num) / posterize_num;
      uint32_t post_b = (1<<15) * ROUND(b * posterize_num) / posterize_num;
      
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      rgba[0] = (opa_a*post_r + opa_b*rgba[0])/(1<<15);
      rgba[1] = (opa_a*post_g + opa_b*rgba[1])/(1<<15);
      rgba[2] = (opa_a*post_b + opa_b*rgba[2])/(1<<15);

    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

// Colorize: apply the source hue and saturation, retaining the target
// brightness. Same thing as in the PDF spec addendum, and upcoming SVG
// compositing drafts. Colorize should be used at either 1.0 or 0.0, values in
// between probably aren't very useful. This blend mode retains the target
// alpha, and any pure whites and blacks in the target layer.

#define MAX3(a, b, c) ((a)>(b)?MAX((a),(c)):MAX((b),(c)))
#define MIN3(a, b, c) ((a)<(b)?MIN((a),(c)):MIN((b),(c)))

// For consistency, these are the values used by MyPaint's Color and
// Luminosity layer blend modes, which in turn are defined by
// http://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html.
// Same as ITU Rec. BT.601 (SDTV) rounded to 2 decimal places.

static const float LUMA_RED_COEFF   = 0.2126 * (1<<15);
static const float LUMA_GREEN_COEFF = 0.7152 * (1<<15);
static const float LUMA_BLUE_COEFF  = 0.0722 * (1<<15);

// See also http://en.wikipedia.org/wiki/YCbCr


/* Returns the sRGB luminance of an RGB triple, expressed as scaled ints. */

#define LUMA(r,g,b) \
   ((r)*LUMA_RED_COEFF + (g)*LUMA_GREEN_COEFF + (b)*LUMA_BLUE_COEFF)


/*
 * Sets the output RGB triple's luminance to that of the input, retaining its
 * colour. Inputs and outputs are scaled ints having factor 2**-15, and must
 * not store premultiplied alpha.
 */

inline static void
set_rgb16_lum_from_rgb16(const uint16_t topr,
                         const uint16_t topg,
                         const uint16_t topb,
                         uint16_t *botr,
                         uint16_t *botg,
                         uint16_t *botb)
{
    // Spec: SetLum()
    // Colours potentially can go out of band to both sides, hence the
    // temporary representation inflation.
    const uint16_t botlum = LUMA(*botr, *botg, *botb) / (1<<15);
    const uint16_t toplum = LUMA(topr, topg, topb) / (1<<15);
    const int16_t diff = botlum - toplum;
    int32_t r = topr + diff;
    int32_t g = topg + diff;
    int32_t b = topb + diff;

    // Spec: ClipColor()
    // Clip out of band values
    int32_t lum = LUMA(r, g, b) / (1<<15);
    int32_t cmin = MIN3(r, g, b);
    int32_t cmax = MAX3(r, g, b);
    if (cmin < 0) {
        r = lum + (((r - lum) * lum) / (lum - cmin));
        g = lum + (((g - lum) * lum) / (lum - cmin));
        b = lum + (((b - lum) * lum) / (lum - cmin));
    }
    if (cmax > (1<<15)) {
        r = lum + (((r - lum) * ((1<<15)-lum)) / (cmax - lum));
        g = lum + (((g - lum) * ((1<<15)-lum)) / (cmax - lum));
        b = lum + (((b - lum) * ((1<<15)-lum)) / (cmax - lum));
    }
#ifdef HEAVY_DEBUG
    assert((0 <= r) && (r <= (1<<15)));
    assert((0 <= g) && (g <= (1<<15)));
    assert((0 <= b) && (b <= (1<<15)));
#endif

    *botr = r;
    *botg = g;
    *botb = b;
}


// The method is an implementation of that described in the official Adobe "PDF
// Blend Modes: Addendum" document, dated January 23, 2006; specifically it's
// the "Color" nonseparable blend mode. We do however use different
// coefficients for the Luma value.

void
draw_dab_pixels_BlendMode_Color (uint16_t *mask,
                                 uint16_t *rgba, // b=bottom, premult
                                 uint16_t color_r,  // }
                                 uint16_t color_g,  // }-- a=top, !premult
                                 uint16_t color_b,  // }
                                 uint16_t opacity)
{
  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      // De-premult
      uint16_t r, g, b;
      const uint16_t a = rgba[3];
      r = g = b = 0;
      if (rgba[3] != 0) {
        r = ((1<<15)*((uint32_t)rgba[0])) / a;
        g = ((1<<15)*((uint32_t)rgba[1])) / a;
        b = ((1<<15)*((uint32_t)rgba[2])) / a;
      }

      // Apply luminance
      set_rgb16_lum_from_rgb16(color_r, color_g, color_b, &r, &g, &b);

      // Re-premult
      r = ((uint32_t) r) * a / (1<<15);
      g = ((uint32_t) g) * a / (1<<15);
      b = ((uint32_t) b) * a / (1<<15);

      // And combine as normal.
      uint32_t opa_a = mask[0] * opacity / (1<<15); // topAlpha
      uint32_t opa_b = (1<<15) - opa_a; // bottomAlpha
      rgba[0] = (opa_a*r + opa_b*rgba[0])/(1<<15);
      rgba[1] = (opa_a*g + opa_b*rgba[1])/(1<<15);
      rgba[2] = (opa_a*b + opa_b*rgba[2])/(1<<15);
    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

// This blend mode is used for smudging and erasing.  Smudging
// allows to "drag" around transparency as if it was a color.  When
// smuding over a region that is 60% opaque the result will stay 60%
// opaque (color_a=0.6).  For normal erasing color_a is set to 0.0
// and color_r/g/b will be ignored. This function can also do normal
// blending (color_a=1.0).
//
void draw_dab_pixels_BlendMode_Normal_and_Eraser (uint16_t * mask,
                                                  uint16_t * rgba,
                                                  uint16_t color_r,
                                                  uint16_t color_g,
                                                  uint16_t color_b,
                                                  uint16_t color_a,
                                                  uint16_t opacity) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      opa_a = opa_a * color_a / (1<<15);
      rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);
      rgba[0] = (opa_a*color_r + opa_b*rgba[0])/(1<<15);
      rgba[1] = (opa_a*color_g + opa_b*rgba[1])/(1<<15);
      rgba[2] = (opa_a*color_b + opa_b*rgba[2])/(1<<15);

    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

void draw_dab_pixels_BlendMode_Normal_and_Eraser_Paint (uint16_t * mask,
                                                  uint16_t * rgba,
                                                  uint16_t color_r,
                                                  uint16_t color_g,
                                                  uint16_t color_b,
                                                  uint16_t color_a,
                                                  uint16_t opacity) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      opacity = MAX(opacity, 150);
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      if (rgba[3] <= 0) {
        opa_a = opa_a * color_a / (1<<15);
        rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);
        rgba[0] = (opa_a*color_r + opa_b*rgba[0])/(1<<15);
        rgba[1] = (opa_a*color_g + opa_b*rgba[1])/(1<<15);
        rgba[2] = (opa_a*color_b + opa_b*rgba[2])/(1<<15);
        continue;
      }
      float fac_a = (float)opa_a / (opa_a + opa_b * rgba[3] / (1<<15));
      fac_a *= (float)color_a / (1<<15);
      float fac_b = 1.0 - fac_a;
      float spectral_b[10] = {0};
      rgb_to_spectral((float)rgba[0] / rgba[3], (float)rgba[1] / rgba[3], (float)rgba[2] / rgba[3], spectral_b);

      // convert top to spectral.  Already straight color
      float spectral_a[10] = {0};
      rgb_to_spectral((float)color_r / (1<<15), (float)color_g / (1<<15), (float)color_b / (1<<15), spectral_a);

      // mix to the two spectral colors using WGM
      float spectral_result[10] = {0};
      for (int i=0; i<10; i++) {
        spectral_result[i] = fastpow(spectral_a[i], fac_a) * fastpow(spectral_b[i], fac_b);
      }
      // convert back to RGB
      float rgb_result[3] = {0};
      spectral_to_rgb(spectral_result, rgb_result);
      
      // apply eraser
      opa_a = opa_a * color_a / (1<<15);
      
      // calculate alpha normally
      rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);

      for (int i=0; i<3; i++) {
        rgba[i] =(rgb_result[i] * rgba[3]) + 0.5;
      }

    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

// This is BlendMode_Normal with locked alpha channel.
//
void draw_dab_pixels_BlendMode_LockAlpha (uint16_t * mask,
                                          uint16_t * rgba,
                                          uint16_t color_r,
                                          uint16_t color_g,
                                          uint16_t color_b,
                                          uint16_t opacity) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      
      opa_a *= rgba[3];
      opa_a /= (1<<15);
          
      rgba[0] = (opa_a*color_r + opa_b*rgba[0])/(1<<15);
      rgba[1] = (opa_a*color_g + opa_b*rgba[1])/(1<<15);
      rgba[2] = (opa_a*color_b + opa_b*rgba[2])/(1<<15);
    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};

void draw_dab_pixels_BlendMode_LockAlpha_Paint (uint16_t * mask,
                                          uint16_t * rgba,
                                          uint16_t color_r,
                                          uint16_t color_g,
                                          uint16_t color_b,
                                          uint16_t opacity) {

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      opacity = MAX(opacity, 150);
      uint32_t opa_a = mask[0]*(uint32_t)opacity/(1<<15); // topAlpha
      uint32_t opa_b = (1<<15)-opa_a; // bottomAlpha
      opa_a *= rgba[3];
      opa_a /= (1<<15);
      if (rgba[3] <= 0) {
        rgba[0] = (opa_a*color_r + opa_b*rgba[0])/(1<<15);
        rgba[1] = (opa_a*color_g + opa_b*rgba[1])/(1<<15);
        rgba[2] = (opa_a*color_b + opa_b*rgba[2])/(1<<15);
        continue;
      }
      float fac_a = (float)opa_a / (opa_a + opa_b * rgba[3] / (1<<15));
      float fac_b = 1.0 - fac_a;
      float spectral_b[10] = {0};
      rgb_to_spectral((float)rgba[0] / rgba[3], (float)rgba[1] / rgba[3], (float)rgba[2] / rgba[3], spectral_b);

      // convert top to spectral.  Already straight color
      float spectral_a[10] = {0};
      rgb_to_spectral((float)color_r / (1<<15), (float)color_g / (1<<15), (float)color_b / (1<<15), spectral_a);

      // mix to the two spectral colors using WGM
      float spectral_result[10] = {0};
      for (int i=0; i<10; i++) {
        spectral_result[i] = fastpow(spectral_a[i], fac_a) * fastpow(spectral_b[i], fac_b);
      }
      // convert back to RGB
      float rgb_result[3] = {0};
      spectral_to_rgb(spectral_result, rgb_result);
      rgba[3] = opa_a + opa_b * rgba[3] / (1<<15);

      for (int i=0; i<3; i++) {
        rgba[i] =(rgb_result[i] * rgba[3]) + 0.5;
      }
    }
    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};


// Sum up the color/alpha components inside the masked region.
// Called by get_color().
//
void get_color_pixels_accumulate (uint16_t * mask,
                                  uint16_t * rgba,
                                  float * sum_weight,
                                  float * sum_r,
                                  float * sum_g,
                                  float * sum_b,
                                  float * sum_a,
                                  float paint
                                  ) {


  // Sample the canvas as additive and subtractive
  // According to paint parameter
  // Average the results normally
  // Only sample a random selection of pixels

  float avg_spectral[10] = {0};
  float avg_rgb[3] = {*sum_r, *sum_g, *sum_b};
  if (paint > 0.0f) {
    rgb_to_spectral(*sum_r, *sum_g, *sum_b, avg_spectral);
  }

  while (1) {
    for (; mask[0]; mask++, rgba+=4) {
      // sample at least one pixel but then only 1%
      if (rand() % 100 != 0 && *sum_a > 0.0) {
        continue;
      }
      float a = (float)mask[0] * rgba[3] / (1<<30);
      float alpha_sums = a + *sum_a;
      *sum_weight += (float)mask[0] / (1<<15);
      float fac_a, fac_b;
      fac_a = fac_b = 1.0f;
      if (alpha_sums > 0.0f) {
        fac_a = a / alpha_sums;
        fac_b = 1.0 - fac_a;
      }
      if (paint > 0.0f) {
        float spectral[10] = {0};
        if (rgba[3] > 0) {
          rgb_to_spectral((float)rgba[0] / rgba[3], (float)rgba[1] / rgba[3], (float)rgba[2] / rgba[3], spectral);
          for (int i=0; i<10; i++) {
            avg_spectral[i] = fastpow(spectral[i], fac_a) * fastpow(avg_spectral[i], fac_b);
          }
        }
      }
      if (paint < 1.0f) {
        if (rgba[3] > 0) {
          for (int i=0; i<3; i++) {
            avg_rgb[i] = (float)rgba[i] * fac_a / rgba[3] + (float)avg_rgb[i] * fac_b;
          }
        }
      }
      *sum_a += a;
    }
    float spec_rgb[3] = {0};
    spectral_to_rgb(avg_spectral, spec_rgb);

    *sum_r = spec_rgb[0] * paint + (1.0 - paint) * avg_rgb[0];
    *sum_g = spec_rgb[1] * paint + (1.0 - paint) * avg_rgb[1];
    *sum_b = spec_rgb[2] * paint + (1.0 - paint) * avg_rgb[2];

    if (!mask[1]) break;
    rgba += mask[1];
    mask += 2;
  }
};



