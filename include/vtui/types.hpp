#pragma once
#include <cstdint>

namespace vtui {

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

using Codepoint = char32_t;

struct Coord {
  uint16_t x = 0;
  uint16_t y = 0;

  bool operator==(const Coord& o) const { return x == o.x && y == o.y; }
  bool operator!=(const Coord& o) const { return !(*this == o); }
};

struct Size {
  uint16_t width = 0;
  uint16_t height = 0;

  bool empty() const { return width == 0 || height == 0; }
  bool operator==(const Size& o) const {
    return width == o.width && height == o.height;
  }
  bool operator!=(const Size& o) const { return !(*this == o); }
};

struct Rect {
  Coord pos;
  Size size;

  bool empty() const { return size.empty(); }
  bool contains(Coord p) const;
  Rect intersect(const Rect& other) const;
  bool operator==(const Rect& o) const {
    return pos == o.pos && size == o.size;
  }
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

struct Cell {
  Codepoint ch = 0;
  Color fg = Color::Default;
  Color bg = Color::Default;

  bool operator==(const Cell& o) const {
    return ch == o.ch && fg == o.fg && bg == o.bg;
  }
  bool operator!=(const Cell& o) const { return !(*this == o); }
};

}  // namespace vtui
