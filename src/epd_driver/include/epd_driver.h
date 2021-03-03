/**
 * A high-level library for drawing to an EPD.
 */
#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "esp_attr.h"
#include "epd_waveforms.h"

#include <stdbool.h>
#include <stdint.h>

// minimal draw time in ms for a frame layer,
// which will allow all particles to set properly.
#ifndef MINIMUM_FRAME_TIME
#define MINIMUM_FRAME_TIME 12
#endif

#if defined(CONFIG_EPD_DISPLAY_TYPE_ED097OC4) ||                               \
    defined(CONFIG_EPD_DISPLAY_TYPE_ED097TC2) ||                               \
    defined(CONFIG_EPD_DISPLAY_TYPE_ED097OC4_LQ)
/// Width of the display area in pixels.
#define EPD_WIDTH 1200
/// Height of the display area in pixels.
#define EPD_HEIGHT 825
#elif defined(CONFIG_EPD_DISPLAY_TYPE_ED133UT2)
#define EPD_WIDTH 1600
#define EPD_HEIGHT 1200
#elif defined(CONFIG_EPD_DISPLAY_TYPE_ED060SC4)
/// Width of the display area in pixels.
#define EPD_WIDTH 800
/// Height of the display area in pixels.
#define EPD_HEIGHT 600
#elif defined(CONFIG_EPD_DISPLAY_TYPE_ED047TC1)
/// Width of the display area in pixels.
#define EPD_WIDTH 960
/// Height of the display area in pixels.
#define EPD_HEIGHT 540
#else
#error "no display type defined!"
#endif

/// An area on the display.
typedef struct {
  /// Horizontal position.
  int x;
  /// Vertical position.
  int y;
  /// Area / image width, must be positive.
  int width;
  /// Area / image height, must be positive.
  int height;
} EpdRect;

typedef struct {
  int phases;
  const uint8_t* luts;
  /// If we have timing information for the individual
  /// phases, this is an array of the on-times for each phase.
  /// Otherwise, this is NULL.
  const int* phase_times;
} EpdWaveformPhases;

typedef struct {
  uint8_t type;
  uint8_t temp_ranges;
  EpdWaveformPhases const **range_data;
} EpdWaveformMode;

typedef struct {
  int min;
  int max;
} EpdWaveformTempInterval;

typedef struct {
  uint8_t num_modes;
  uint8_t num_temp_ranges;
  EpdWaveformMode const **mode_data;
  EpdWaveformTempInterval const *temp_intervals;
} EpdWaveform;


/// Possible failures when drawing.
enum EpdDrawError {
  DRAW_SUCCESS = 0x0,
  /// No valid framebuffer packing mode was specified.
  DRAW_INVALID_PACKING_MODE = 0x1,

  /// No lookup table implementation for this mode / packing.
  DRAW_LOOKUP_NOT_IMPLEMENTED = 0x2,

  /// The string to draw is invalid.
  DRAW_STRING_INVALID = 0x4,

  /// The string was not empty, but no characters where drawable.
  DRAW_NO_DRAWABLE_CHARACTERS = 0x8,

  /// Allocation failed
  DRAW_FAILED_ALLOC = 0x10,

  /// A glyph could not be drawn, and not fallback was present.
  DRAW_GLYPH_FALLBACK_FAILED = 0x20,

  /// The specified crop area is invalid.
  DRAW_INVALID_CROP = 0x40,

  /// No such mode is available with the current waveform.
  DRAW_MODE_NOT_FOUND = 0x80,

  /// The waveform info file contains no applicable temperature range.
  DRAW_NO_PHASES_AVAILABLE = 0x100,

  /// An invalid combination of font flags was used.
  DRAW_INVALID_FONT_FLAGS = 0x200,
};

/// Global EPD driver options.
enum EpdInitOptions {
    /// Use the default options.
    EPD_OPTIONS_DEFAULT = 0,
    /// Use a small look-up table of 1024 bytes.
    /// The EPD driver will use less space, but performance may be worse.
    EPD_LUT_1K = 1,
    /// Use a 64K lookup table. (default)
    /// Best performance, but permanently occupies a 64k block of internal memory.
    EPD_LUT_64K = 2,

    /// Use a small feed queue of 8 display lines.
    /// This uses less memory, but may impact performance.
    EPD_FEED_QUEUE_8 = 4,
    /// Use a feed queue of 32 display lines. (default)
    /// Best performance, but larger memory footprint.
    EPD_FEED_QUEUE_32 = 8,
};

/// The image drawing mode.
enum EpdDrawMode {
  /// "Official" Waveform modes.
  MODE_INIT = 0x0,
  MODE_DU = 0x1,
  MODE_GC16 = 0x2,
  MODE_GC16_FAST = 0x3,
  MODE_A2 = 0x4,
  MODE_GL16 = 0x5,
  MODE_GL16_FAST = 0x6,
  MODE_DU4 = 0x7,
  MODE_GL4 = 0xA,
  MODE_GL16_INV = 0xB,

  /// Extended Waveform modes.
  MODE_EPDIY_WHITE_TO_GL16 = 0x10,
  MODE_EPDIY_BLACK_TO_GL16 = 0x11,

  /// Monochrome mode. Only supported with 1bpp buffers.
  MODE_EPDIY_MONOCHROME = 0x20,

  MODE_UNKNOWN_WAVEFORM = 0x3F,

  /// Use a vendor waveform
  //VENDOR_WAVEFORM = 0x10,
  /// Use EPDIY built-in waveform
  //EPDIY_WAVEFORM = 0x20,

  // Framebuffer packing modes
  /// 1 bit-per-pixel framebuffer with 0 = black, 1 = white.
  /// MSB is left is the leftmost pixel, LSB the rightmost pixel.
  MODE_PACKING_8PPB = 0x40,
  /// 4 bit-per pixel framebuffer with 0x0 = black, 0xF = white.
  /// The upper nibble corresponds to the left pixel.
  MODE_PACKING_2PPB = 0x80,
  /// A difference image with one pixel per byte.
  /// The upper nibble marks the "from" color,
  /// the lower nibble the "to" color.
  MODE_PACKING_1PPB_DIFFERENCE = 0x100,
  // reserver for 4PPB mode

  /// Draw on monochrome background color
  /// Draw on a white background
  PREVIOUSLY_WHITE = 0x200,
  /// Draw on a black background
  PREVIOUSLY_BLACK = 0x400,

  /// Invert colors in the framebuffer (0xF = black, 0x0 = White).
  INVERT = 0x800,
};

#define EPD_MODE_DEFAULT (MODE_GL16 | PREVIOUSLY_WHITE)

/// Font drawing flags
enum EpdFontFlags {
  /// Draw a background.
  ///
  /// Take the background into account
  /// when calculating the size.
  DRAW_BACKGROUND = 0x1,

  /// Left-Align lines
  DRAW_ALIGN_LEFT = 0x2,
  /// Right-align lines
  DRAW_ALIGN_RIGHT = 0x4,
  /// Center-align lines
  DRAW_ALIGN_CENTER = 0x8,
};

/// Font properties.
typedef struct {
  /// Foreground color
  uint8_t fg_color : 4;
  /// Background color
  uint8_t bg_color : 4;
  /// Use the glyph for this codepoint for missing glyphs.
  uint32_t fallback_glyph;
  /// Additional flags, reserved for future use
  enum EpdFontFlags flags;
} EpdFontProperties;

/** Initialize the ePaper display */
void epd_init(enum EpdInitOptions options);

/** Deinit the ePaper display */
void epd_deinit();

/** Enable display power supply. */
void epd_poweron();

/** Disable display power supply. */
void epd_poweroff();

/** Clear the whole screen by flashing it. */
void epd_clear();

/**
 * Clear an area by flashing it.
 *
 * @param area: The area to clear.
 */
void epd_clear_area(EpdRect area);

/**
 * Clear an area by flashing it.
 *
 * @param area: The area to clear.
 * @param cycles: The number of black-to-white clear cycles.
 * @param cycle_time: Length of a cycle. Default: 50 (us).
 */
void epd_clear_area_cycles(EpdRect area, int cycles, int cycle_time);

/**
 * Darken / lighten an area for a given time.
 *
 * @param area: The area to darken / lighten.
 * @param time: The time in us to apply voltage to each pixel.
 * @param color: 1: lighten, 0: darken.
 */
void epd_push_pixels(EpdRect area, short time, int color);

/**
 * Draw a picture to a given area. The image area is not cleared and assumed
 * to be white before drawing.
 *
 * @param area: The display area to draw to. `width` and `height` of the area
 *   must correspond to the image dimensions in pixels.
 * @param data: The image data, as a buffer of 4 bit wide brightness values.
 *   Pixel data is packed (two pixels per byte). A byte cannot wrap over
 * multiple rows, images of uneven width must add a padding nibble per line.
 */
void IRAM_ATTR epd_draw_grayscale_image(EpdRect area, const uint8_t *data);

/**
 * Draw a picture to a given area, with some draw mode.
 * The image area is not cleared before drawing.
 * For example, this can be used for pixel-aligned clearing.
 *
 * @param area: The display area to draw to. `width` and `height` of the area
 *   must correspond to the image dimensions in pixels.
 * @param data: The image data, as a buffer of 4 bit wide brightness values.
 *   Pixel data is packed (two pixels per byte). A byte cannot wrap over
 * multiple rows, images of uneven width must add a padding nibble per line.
 * @param mode: Configure image color and assumptions of the display state.
 */
//void IRAM_ATTR epd_draw_image(EpdRect area, const uint8_t *data,
//                              enum EpdDrawMode mode);

/**
 * FIXME: update
 * Same as epd_draw_image, but with the option to specify
 * which lines of the image should be drawn.
 *
 * @param drawn_lines: If not NULL, an array of at least the height of the
 * image. Every line where the corresponding value in `lines` is `false` will be
 * skipped.
 * @param data: The image data, as a buffer of 4 bit wide brightness values.
 *   Pixel data is packed (two pixels per byte). A byte cannot wrap over
 * multiple rows, images of uneven width must add a padding nibble per line.
 * @param mode: Configure image color and assumptions of the display state.
 * @param drawn_lines: Optional line mask.
 *   If not NULL, only draw lines which are marked as `true`.
 */
enum EpdDrawError IRAM_ATTR epd_draw_base(EpdRect area,
                            const uint8_t *data,
                            EpdRect crop_to,
                            enum EpdDrawMode mode,
                            int temperature,
                            const bool *drawn_lines,
                            const EpdWaveform *waveform);

enum EpdDrawError epd_draw_image(EpdRect area, const uint8_t *data, const EpdWaveform *waveform);

EpdRect epd_difference_image(const uint8_t* to, const uint8_t* from, uint8_t* interlaced, bool* dirty_lines);

EpdRect epd_difference_image_cropped(
    const uint8_t* to,
    const uint8_t* from,
    EpdRect crop_to,
    uint8_t* interlaced,
    bool* dirty_lines,
    bool* previously_white,
    bool* previously_black
);

/**
 * @returns Rectancle representing the whole screen area.
 */
EpdRect epd_full_screen();

/**
 * Draw a picture to a given framebuffer.
 *
 * @param image_area: The area to copy to. `width` and `height` of the area
 *   must correspond to the image dimensions in pixels.
 * @param image_data: The image data, as a buffer of 4 bit wide brightness
 * values. Pixel data is packed (two pixels per byte). A byte cannot wrap over
 * multiple rows, images of uneven width must add a padding nibble per line.
 * @param framebuffer: The framebuffer object,
 *   which must be `EPD_WIDTH / 2 * EPD_HEIGHT` large.
 */
void epd_copy_to_framebuffer(EpdRect image_area, const uint8_t *image_data,
                             uint8_t *framebuffer);

/**
 * Draw a pixel a given framebuffer.
 *
 * @param x: Horizontal position in pixels.
 * @param y: Vertical position in pixels.
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_draw_pixel(int x, int y, uint8_t color, uint8_t *framebuffer);

/**
 * Draw a horizontal line to a given framebuffer.
 *
 * @param x: Horizontal start position in pixels.
 * @param y: Vertical start position in pixels.
 * @param length: Length of the line in pixels.
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 *  which must be `EPD_WIDTH / 2 * EPD_HEIGHT` bytes large.
 */
void epd_draw_hline(int x, int y, int length, uint8_t color,
                    uint8_t *framebuffer);

/**
 * Draw a horizontal line to a given framebuffer.
 *
 * @param x: Horizontal start position in pixels.
 * @param y: Vertical start position in pixels.
 * @param length: Length of the line in pixels.
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 *  which must be `EPD_WIDTH / 2 * EPD_HEIGHT` bytes large.
 */
void epd_draw_vline(int x, int y, int length, uint8_t color,
                    uint8_t *framebuffer);

void epd_fill_circle_helper(int x0, int y0, int r, int corners, int delta,
                            uint8_t color, uint8_t *framebuffer);

/**
 * Draw a circle with given center and radius
 *
 * @param x: Center-point x coordinate
 * @param y: Center-point y coordinate
 * @param r: Radius of the circle in pixels
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_draw_circle(int x, int y, int r, uint8_t color, uint8_t *framebuffer);

/**
 * Draw a circle with fill with given center and radius
 *
 * @param x: Center-point x coordinate
 * @param y: Center-point y coordinate
 * @param r: Radius of the circle in pixels
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_fill_circle(int x, int y, int r, uint8_t color, uint8_t *framebuffer);

/**
 * Draw a rectanle with no fill color
 *
 * @param x: Top left corner x coordinate
 * @param y: Top left corner y coordinate
 * @param w: Width in pixels
 * @param h: Height in pixels
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_draw_rect(EpdRect rect, uint8_t color, uint8_t *framebuffer);

/**
 * Draw a rectanle with fill color
 *
 * @param x: Top left corner x coordinate
 * @param y: Top left corner y coordinate
 * @param w: Width in pixels
 * @param h: Height in pixels
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_fill_rect(EpdRect rect, uint8_t color, uint8_t *framebuffer);

/**
 * Draw a line
 *
 * @param    x0  Start point x coordinate
 * @param    y0  Start point y coordinate
 * @param    x1  End point x coordinate
 * @param    y1  End point y coordinate
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_draw_line(int x0, int y0, int x1, int y1, uint8_t color,
                   uint8_t *framebuffer);

/**
 * Draw a triangle with no fill color
 *
 * @param    x0  Vertex #0 x coordinate
 * @param    y0  Vertex #0 y coordinate
 * @param    x1  Vertex #1 x coordinate
 * @param    y1  Vertex #1 y coordinate
 * @param    x2  Vertex #2 x coordinate
 * @param    y2  Vertex #2 y coordinate
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                       uint8_t color, uint8_t *framebuffer);

/**
 * Draw a triangle with color-fill
 *
 * @param    x0  Vertex #0 x coordinate
 * @param    y0  Vertex #0 y coordinate
 * @param    x1  Vertex #1 x coordinate
 * @param    y1  Vertex #1 y coordinate
 * @param    x2  Vertex #2 x coordinate
 * @param    y2  Vertex #2 y coordinate
 * @param color: The gray value of the line (0-255);
 * @param framebuffer: The framebuffer to draw to,
 */
void epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                       uint8_t color, uint8_t *framebuffer);
/**
 * Get the current ambient temperature in °C.
 */
float epd_ambient_temperature();

/// Font data stored PER GLYPH
typedef struct {
  uint8_t width;            ///< Bitmap dimensions in pixels
  uint8_t height;           ///< Bitmap dimensions in pixels
  uint8_t advance_x;        ///< Distance to advance cursor (x axis)
  int16_t left;             ///< X dist from cursor pos to UL corner
  int16_t top;              ///< Y dist from cursor pos to UL corner
  uint16_t compressed_size; ///< Size of the zlib-compressed font data.
  uint32_t data_offset;     ///< Pointer into EpdFont->bitmap
} EpdGlyph;

/// Glyph interval structure
typedef struct {
  uint32_t first;  ///< The first unicode code point of the interval
  uint32_t last;   ///< The last unicode code point of the interval
  uint32_t offset; ///< Index of the first code point into the glyph array
} EpdUnicodeInterval;

/// Data stored for FONT AS A WHOLE
typedef struct {
  const uint8_t *bitmap;            ///< Glyph bitmaps, concatenated
  const EpdGlyph *glyph;            ///< Glyph array
  const EpdUnicodeInterval *intervals; ///< Valid unicode intervals for this font
  uint32_t interval_count;    ///< Number of unicode intervals.
  bool compressed;            ///< Does this font use compressed glyph bitmaps?
  uint8_t advance_y;          ///< Newline distance (y axis)
  int ascender;               ///< Maximal height of a glyph above the base line
  int descender;              ///< Maximal height of a glyph below the base line
} EpdFont;

/*!
 * Get the text bounds for string, when drawn at (x, y).
 * Set font properties to NULL to use the defaults.
 */
void epd_get_text_bounds(const EpdFont *font, const char *string,
                     const int *x, const int *y,
                     int *x1, int *y1, int *w, int *h,
                     const EpdFontProperties *props);

/**
 * Write text to the EPD.
 */
enum EpdDrawError epd_write_string(const EpdFont *font, const char *string, int *cursor_x,
                int *cursor_y, uint8_t *framebuffer,
                const EpdFontProperties *properties);

/**
 * Write a (multi-line) string to the EPD.
 */
enum EpdDrawError epd_write_default(const EpdFont *font, const char *string, int *cursor_x,
                  int *cursor_y, uint8_t *framebuffer);

/**
 * Get the font glyph for a unicode code point.
 */
const EpdGlyph* epd_get_glyph(const EpdFont *font, uint32_t code_point);

/**
 * The default font properties.
 */
EpdFontProperties epd_font_properties_default();

#ifdef __cplusplus
}
#endif