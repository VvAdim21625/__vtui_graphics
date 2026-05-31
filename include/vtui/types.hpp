/**
 * @mainpage vtui — Terminal UI graphics library
 *
 * Double-buffered screen rendering with dirty-tracked incremental updates,
 * ANSI escape sequence output, and platform abstraction.
 *
 * Key types are defined in @ref types.hpp, the buffer in @ref buffer.hpp,
 * the renderer in @ref renderer.hpp, and result in @ref result.hpp.
 * Platform-specific code lives under src/platform/.
 */

#pragma once
#include <cstdint>

namespace vtui {

/** @brief 256-colour palette values (indices 0–15 + Default). */
enum class Color : uint8_t {
  Black = 0,
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Magenta = 5,
  Cyan = 6,
  LightGray = 7,
  DarkGray = 8,
  LightRed = 9,
  LightGreen = 10,
  LightYellow = 11,
  LightBlue = 12,
  LightMagenta = 13,
  LightCyan = 14,
  White = 15,
  Default = 255,
};

/// Unicode codepoint (UTF-32).
using Codepoint = char32_t;

/// Maximum number of extension codepoints in a grapheme cluster, stored
/// in Cell::ext[].  Total codepoints per cluster = 1 (base) + ext_len.
static constexpr uint8_t MaxClusterExt = 7;

/** @brief 2D coordinate (column, row). */
struct Coord {
  uint16_t x = 0;
  uint16_t y = 0;

  /// True if both x and y are equal.
  bool operator==(const Coord& o) const { return x == o.x && y == o.y; }
  /// True if either x or y differs.
  bool operator!=(const Coord& o) const { return !(*this == o); }
};

/** @brief Width and height in cells.  Zero in either dimension means empty. */
struct Size {
  uint16_t width = 0;
  uint16_t height = 0;

  /// True if either dimension is zero.
  bool empty() const { return width == 0 || height == 0; }
  /// True if both width and height are equal.
  bool operator==(const Size& o) const {
    return width == o.width && height == o.height;
  }
  /// True if width or height differ.
  bool operator!=(const Size& o) const { return !(*this == o); }
};

/** @brief Axis-aligned rectangle defined by origin + size. */
struct Rect {
  Coord pos;
  Size size;

  /// True if the rectangle has zero width or height.
  bool empty() const { return size.empty(); }
  /// True if point @p p lies within (or on the boundary of) the rectangle.
  bool contains(Coord p) const;
  /// Return the intersection of two rectangles (empty rect if disjoint).
  Rect intersect(const Rect& other) const;
  /// True if both origin and size are equal.
  bool operator==(const Rect& o) const {
    return pos == o.pos && size == o.size;
  }
  /// True if origin or size differ.
  bool operator!=(const Rect& o) const { return !(*this == o); }
};

inline bool Rect::contains(Coord p) const {
  return p.x >= pos.x && p.x < pos.x + size.width && p.y >= pos.y &&
         p.y < pos.y + size.height;
}

inline Rect Rect::intersect(const Rect& other) const {
  uint16_t x1 = pos.x > other.pos.x ? pos.x : other.pos.x;
  uint16_t y1 = pos.y > other.pos.y ? pos.y : other.pos.y;
  uint16_t x2 = pos.x + size.width < other.pos.x + other.size.width
                    ? pos.x + size.width
                    : other.pos.x + other.size.width;
  uint16_t y2 = pos.y + size.height < other.pos.y + other.size.height
                    ? pos.y + size.height
                    : other.pos.y + other.size.height;

  if (x1 >= x2 || y1 >= y2)
    return {{0, 0}, {0, 0}};

  return {{x1, y1},
          {static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)}};
}

/** @brief A single terminal cell: codepoint + foreground/background colours. */
struct Cell {
  Codepoint base = 0;
  Codepoint ext[MaxClusterExt] = {};
  uint8_t ext_len = 0;
  uint8_t width = 1;
  uint8_t trail = 0;
  Color fg = Color::Default;
  Color bg = Color::Default;

  void compute_width();

  /// True if both codepoint and colours are equal.
  bool operator==(const Cell& o) const {
    if (base != o.base || ext_len != o.ext_len || fg != o.fg || bg != o.bg)
      return false;

    for (uint8_t i = 0; i < ext_len; ++i) {
      if (ext[i] != o.ext[i])
        return false;
    }
    return true;
  }
  /// True if codepoint or colours differ.
  bool operator!=(const Cell& o) const { return !(*this == o); }
};

}  // namespace vtui
