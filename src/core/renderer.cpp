#include "vtui/renderer.hpp"

#include <cstdio>

#include "../util/utf8.h"
#include "platform/pal.h"

namespace vtui {
namespace {

constexpr char ANSI_CURSOR_FMT[] = "\x1b[%d;%dH";
constexpr char ANSI_COLOR_FMT[] = "\x1b[38;5;%d;48;5;%dm";

}  // namespace

Renderer::Renderer() : last_cursor_(InvalidCursor) {}

Renderer::~Renderer() {
  flush();
}

Result<void> Renderer::flush() {
  if (len_ > 0) {
    if (!vtui_pal_write_output(reinterpret_cast<const uint8_t*>(buf_),
                               static_cast<int>(len_)))
      return {Err, Errc::IOError};
    len_ = 0;
  }
  return {};
}

bool Renderer::would_overflow(size_t extra) const {
  return len_ + extra >= BufSize - FlushThreshold;
}

void Renderer::append(char c) {
  if (len_ >= BufSize - 1)
    flush();
  buf_[len_++] = c;
}

void Renderer::append_str(const char* s, size_t n) {
  if (len_ + n >= BufSize - FlushThreshold)
    flush();
  for (size_t i = 0; i < n; ++i)
    buf_[len_++] = s[i];
}

void Renderer::append_utf8(Codepoint cp) {
  char tmp[5];
  vtui_utf8_encode(cp, tmp);
  size_t n = 0;
  while (tmp[n])
    ++n;
  append_str(tmp, n);
}

void Renderer::append_cursor(Coord pos) {
  char tmp[32];
  int n =
      std::snprintf(tmp, sizeof(tmp), ANSI_CURSOR_FMT, pos.y + 1, pos.x + 1);
  if (n > 0)
    append_str(tmp, static_cast<size_t>(n));
}

void Renderer::append_color(Color fg, Color bg) {
  char tmp[32];
  int n = std::snprintf(tmp, sizeof(tmp), ANSI_COLOR_FMT, static_cast<int>(fg),
                        static_cast<int>(bg));
  if (n > 0)
    append_str(tmp, static_cast<size_t>(n));
}

Result<void> Renderer::present(ScreenBuffer& buffer) {
  if (buffer.empty())
    return {};

  Size size = buffer.size();

  for (uint16_t y = 0; y < size.height; ++y) {
    for (uint16_t x = 0; x < size.width; ++x) {
      Coord cursor = {x, y};
      size_t idx = static_cast<size_t>(y) * size.width + x;

      if (!buffer.is_dirty(idx))
        continue;

      const Cell& back = buffer.at(cursor);
      Cell& front = buffer.front(cursor);

      if (front == back)
        continue;

      if (would_overflow(32))
        flush();

      if (last_cursor_ == InvalidCursor || x != last_cursor_.x + 1 ||
          y != last_cursor_.y)
        append_cursor(cursor);

      if (back.fg != last_fg_ || back.bg != last_bg_) {
        append_color(back.fg, back.bg);
        last_fg_ = back.fg;
        last_bg_ = back.bg;
      }

      append_utf8(back.ch);

      last_cursor_ = cursor;
      front = back;
    }
  }

  flush();
  buffer.clear_dirty();
  last_cursor_ = InvalidCursor;

  return {};
}

}  // namespace vtui
