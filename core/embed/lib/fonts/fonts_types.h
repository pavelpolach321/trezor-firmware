#ifndef _FONTS_TYPES_H
#define _FONTS_TYPES_H

#include <stdint.h>

/// Font information structure containing metadata and pointers to font data
typedef struct {
  int height;
  int max_height;
  int baseline;
  const uint8_t* const* glyph_data;
  const uint8_t* glyph_nonprintable;
} font_info_t;

/// Font identifiers
typedef enum {
  FONT_NORMAL = 0,
  FONT_NORMAL_UPPER,
  FONT_BOLD,
  FONT_BOLD_UPPER,
  FONT_DEMIBOLD,
  FONT_MONO,
  FONT_BIG,
  FONT_SUB,

  FONTS_COUNT  // Keep this last to know the number of fonts
} font_id_t;

/// Font glyph iterator structure
typedef struct {
  const int font;
  const uint8_t* text;
  int remaining;
} font_glyph_iter_t;

#endif  // _FONTS_TYPES_H
