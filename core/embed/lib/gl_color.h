/*
 * This file is part of the Trezor project, https://trezor.io/
 *
 * Copyright (c) SatoshiLabs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GL_COLOR_H
#define GL_COLOR_H

#include <stdint.h>

#define GL_COLOR_16BIT
// #define GL_COLOR_32BIT

// Color in RGB565 format
//
// |15            8 | 7             0|
// |---------------------------------|
// |r r r r r g g g | g g g b b b b b|
// |---------------------------------|

typedef uint16_t gl_color16_t;

// Color in RGBA8888 format
//
// |31           24 |23            16 |15             8 | 7             0 |
// |----------------------------------------------------------------------|
// |a a a a a a a a | r r r r r r r r | g g g g g g g g | b b b b b b b b |
// |----------------------------------------------------------------------|
//

typedef uint32_t gl_color32_t;

#ifdef GL_COLOR_16BIT
#define gl_color_t gl_color16_t
#define gl_color_to_color16(c) (c)
#define gl_color16_to_color(c) (c)
#define gl_color_to_color32(c) (gl_color16_to_color32(c))
#define gl_color32_to_color(c) (gl_color32_to_color16(c))
#define gl_color_lum(c) (gl_color16_lum(c))
#elif GL_COLOR_32BIT
#define gl_color_t gl_color32_t
#define gl_color_to_color16(c) (gl_color32_to_color16(c))
#define gl_color16_to_color(c) (gl_color16_to_color32(c))
#define gl_color_to_color32(c) (c)
#define gl_color32_to_color(c) (c)
#define gl_color_lum(c) (gl_color32_lum(c))
#else
#error "GL_COLOR_16BIT/32BIT not specified"
#endif

// Extracts red component from gl_color16_t and converts it to 8-bit value
#define gl_color16_to_r(c) ((((c)&0xF800) >> 8) | (((c)&0xF800) >> 13))
// Extracts green component from gl_color16_t and converts it to 8-bit value
#define gl_color16_to_g(c) ((((c)&0x07E0) >> 3) | (((c)&0x07E0) >> 9))
// Extracts blue component from gl_color16_t and converts it to 8-bit value
#define gl_color16_to_b(c) ((((c)&0x001F) << 3) | (((c)&0x001F) >> 2))

// Extracts red component from gl_color32_t
#define gl_color32_to_r(c) (((c)&0x00FF0000) >> 16)
// Extracts green component from gl_color32_t
#define gl_color32_to_g(c) (((c)&0x0000FF00) >> 8)
// Extracts blue component from gl_color32_t
#define gl_color32_to_b(c) (((c)&0x000000FF) >> 0)

// 4-bit linear interpolation between `fg` and `bg`
#define a4_lerp(fg, bg, alpha) (((fg) * (alpha) + ((bg) * (15 - (alpha)))) / 15)
// 8-bit linear interpolation between `fg` and `bg`
#define a8_lerp(fg, bg, alpha) \
  (((fg) * (alpha) + ((bg) * (255 - (alpha)))) / 255)

// Constructs a 16-bit color from the given red (r),
// green (g), and blue (b) values in the range 0..255
static inline gl_color16_t gl_color16_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8U) << 8) | ((g & 0xFCU) << 3) | ((b & 0xF8U) >> 3);
}

// Constructs a 32-bit color from the given red (r),
// green (g), and blue (b) values in the range 0..255.
// Alpha is set to 255.
static inline gl_color32_t gl_color32_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFFU << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Converts a 16-bit color to a 32-bit color; alpha is set to 255
static inline gl_color32_t gl_color16_to_color32(gl_color16_t color) {
  uint32_t r = gl_color16_to_r(color);
  uint32_t g = gl_color16_to_g(color);
  uint32_t b = gl_color16_to_b(color);

  return gl_color32_rgb(r, g, b);
}

// Converts 32-bit color to 16-bit color, alpha is ignored
static inline gl_color16_t gl_color32_to_color16(gl_color32_t color) {
  uint16_t r = (color & 0x00F80000) >> 8;
  uint16_t g = (color & 0x0000FC00) >> 5;
  uint16_t b = (color & 0x000000F8) >> 3;

  return r | g | b;
}

// Converts 16-bit color into luminance (ranging from 0 to 255)
static inline uint8_t gl_color16_lum(gl_color16_t color) {
  uint32_t r = gl_color16_to_r(color);
  uint32_t g = gl_color16_to_g(color);
  uint32_t b = gl_color16_to_b(color);

  return (r + g + b) / 3;
}

// Converts 32-bit color into luminance (ranging from 0 to 255)
static inline uint8_t gl_color32_lum(gl_color16_t color) {
  uint32_t r = gl_color32_to_r(color);
  uint32_t g = gl_color32_to_g(color);
  uint32_t b = gl_color32_to_b(color);

  return (r + g + b) / 3;
}

#ifdef GL_COLOR_16BIT
// Blends foreground and background colors with 4-bit alpha
//
// Returns a color in 16-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 15, the function returns the foreground color
static inline gl_color16_t gl_color16_blend_a4(gl_color16_t fg, gl_color16_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = (fg & 0xF800) >> 11;
  uint16_t bg_r = (bg & 0xF800) >> 11;
  uint16_t r = a4_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = (fg & 0x07E0) >> 5;
  uint16_t bg_g = (bg & 0x07E0) >> 5;
  uint16_t g = a4_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = (fg & 0x001F) >> 0;
  uint16_t bg_b = (bg & 0x001F) >> 0;
  uint16_t b = a4_lerp(fg_b, bg_b, alpha);

  return (r << 11) | (g << 5) | b;
}

// Blends foreground and background colors with 8-bit alpha
//
// Returns a color in 16-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 15, the function returns the foreground color
static inline gl_color16_t gl_color16_blend_a8(gl_color16_t fg, gl_color16_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = (fg & 0xF800) >> 11;
  uint16_t bg_r = (bg & 0xF800) >> 11;
  uint16_t r = a8_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = (fg & 0x07E0) >> 5;
  uint16_t bg_g = (bg & 0x07E0) >> 5;
  uint16_t g = a8_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = (fg & 0x001F) >> 0;
  uint16_t bg_b = (bg & 0x001F) >> 0;
  uint16_t b = a8_lerp(fg_b, bg_b, alpha);

  return (r << 11) | (g << 5) | b;
}

// Blends foreground and background colors with 4-bit alpha
//
// Returns a color in 32-bit format
//
// If alpha is 0, the function returns the background color
// If alpha is 15, the function returns the foreground color
static inline gl_color32_t gl_color32_blend_a4(gl_color16_t fg, gl_color16_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = gl_color16_to_r(fg);
  uint16_t bg_r = gl_color16_to_r(bg);
  uint16_t r = a4_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = gl_color16_to_g(fg);
  uint16_t bg_g = gl_color16_to_g(bg);
  uint16_t g = a4_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = gl_color16_to_b(fg);
  uint16_t bg_b = gl_color16_to_b(bg);
  uint16_t b = a4_lerp(fg_b, bg_b, alpha);

  return gl_color32_rgb(r, g, b);
}

// Blends foreground and background colors with 8-bit alpha
//
// Returns a color in 32-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 255, the function returns the foreground color
static inline gl_color32_t gl_color32_blend_a8(gl_color16_t fg, gl_color16_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = gl_color16_to_r(fg);
  uint16_t bg_r = gl_color16_to_r(bg);
  uint16_t r = a8_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = gl_color16_to_g(fg);
  uint16_t bg_g = gl_color16_to_g(bg);
  uint16_t g = a8_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = gl_color16_to_b(fg);
  uint16_t bg_b = gl_color16_to_b(bg);
  uint16_t b = a8_lerp(fg_b, bg_b, alpha);

  return gl_color32_rgb(r, g, b);
}

#elif GL_COLOR_32BIT

// Blends foreground and background colors with 4-bit alpha
//
// Returns a color in 16-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 15, the function returns the foreground color
static inline gl_color16_t gl_color16_blend_a4(gl_color32_t fg, gl_color32_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = gl_color32_to_r(fg);
  uint16_t bg_r = gl_color32_to_r(bg);
  uint16_t r = a4_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = gl_color32_to_g(fg);
  uint16_t bg_g = gl_color32_to_g(bg);
  uint16_t g = a4_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = gl_color32_to_b(fg);
  uint16_t bg_b = gl_color32_to_b(bg);
  uint16_t b = a4_lerp(fg_b, bg_b, alpha);

  return gl_color16_rgb(r, g, b);
}

// Blends foreground and background colors with 8-bit alpha
//
// Returns a color in 16-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 255, the function returns the foreground color
static inline gl_color16_t gl_color16_blend_a8(gl_color32_t fg, gl_color32_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = gl_color32_to_r(fg);
  uint16_t bg_r = gl_color32_to_r(bg);
  uint16_t r = a8_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = gl_color32_to_g(fg);
  uint16_t bg_g = gl_color32_to_g(bg);
  uint16_t g = a8_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = gl_color32_to_b(fg);
  uint16_t bg_b = gl_color32_to_b(bg);
  uint16_t b = g = a8_lerp(fg_b, bg_b, alpha);

  return gl_color16_rgb(r, g, b);
}

// Blends foreground and background colors with 4-bit alpha
//
// Returns a color in 32-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 15, the function returns the foreground color
static inline gl_color32_t gl_color32_blend_a4(gl_color32_t fg, gl_color32_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = gl_color32_to_r(fg);
  uint16_t bg_r = gl_color32_to_r(bg);
  uint16_t r = a4_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = gl_color32_to_g(fg);
  uint16_t bg_g = gl_color32_to_g(bg);
  uint16_t g = a4_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = gl_color32_to_b(fg);
  uint16_t bg_b = gl_color32_to_b(bg);
  uint16_t b = a4_lerp(fg_b, bg_b, alpha);

  return gl_color32_rgb(r, g, b);
}

// Blends foreground and background colors with 8-bit alpha
//
// Returns a color in 32-bit format
//
// If `alpha` is 0, the function returns the background color
// If `alpha` is 15, the function returns the foreground color
static inline gl_color32_t gl_color32_blend_a8(gl_color32_t fg, gl_color32_t bg,
                                               uint8_t alpha) {
  uint16_t fg_r = gl_color32_to_r(fg);
  uint16_t bg_r = gl_color32_to_r(bg);
  uint16_t r = a8_lerp(fg_r, bg_r, alpha);

  uint16_t fg_g = gl_color32_to_g(fg);
  uint16_t bg_g = gl_color32_to_g(bg);
  uint16_t g = a8_lerp(fg_g, bg_g, alpha);

  uint16_t fg_b = gl_color32_to_b(fg);
  uint16_t bg_b = gl_color32_to_b(bg);
  uint16_t b = a8_lerp(fg_b, bg_b, alpha);

  return gl_color32_rgb(r, g, b);
}

#else
#error "GL_COLOR_16BIT/32BIT not specified"
#endif

// Returns a gradient as an array of 16 consecutive 16-bit colors
//
// Each element in the array represents a color, with `retval[0]` being
// the background (`bg`) color and `retval[15]` the foreground (`fg`) color
const gl_color16_t* gl_color16_gradient_a4(gl_color_t fg, gl_color_t bg);

// Returns a gradient as an array of 16 consecutive 32-bit colors
//
// Each element in the array represents a color, with `retval[0]` being
// the background (`bg`) color and `retval[15]` the foreground (`fg`) color
const gl_color32_t* gl_color32_gradient_a4(gl_color_t fg, gl_color_t bg);

#endif  // TREZORHAL_GL_COLOR_H
