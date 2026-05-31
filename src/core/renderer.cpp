#include "vtui/renderer.hpp"

#include <cstdio>

#include "platform/pal.hpp"
#include "util/grapheme.hpp"

namespace vtui {
namespace {

/// ANSI escape sequence for cursor positioning: CUP (row, column).
constexpr char ANSI_CURSOR_FMT[] = "\x1b[%d;%dH";

/// ANSI escape sequence for foreground and background colour via 256-colour
/// SGR: "\e[38;5;fg;48;5;bgm".
constexpr char ANSI_COLOR_FMT[] = "\x1b[38;5;%d;48;5;%dm";

}  // namespace

/** @copydoc Renderer::Renderer */
Renderer::Renderer() : last_cursor_(InvalidCursor) {}

/** @copydoc Renderer::~Renderer */
Renderer::~Renderer() {
  flush();
}

/** @copydoc Renderer::flush */
Result<void> Renderer::flush() {
  if (len_ > 0) {
    if (!pal::write_output(reinterpret_cast<const uint8_t*>(buf_),
                           static_cast<int>(len_)))
      return {Err, Errc::IOError};
    len_ = 0;
  }
  return {};
}

/**
 * @brief Check whether adding extra bytes would cross the flush threshold.
 *
 * The threshold (BufSize - FlushThreshold) ensures there is always room for
 * the largest single ANSI sequence without exceeding the buffer.
 */
bool Renderer::would_overflow(size_t extra) const {
  return len_ + extra >= BufSize - FlushThreshold;
}

/** @brief Append a single byte to the buffer, flushing if it would overflow. */
void Renderer::append(char c) {
  if (len_ >= BufSize - 1)
    flush();
  buf_[len_++] = c;
}

/** @brief Append a span of bytes to the buffer, auto-flushing as needed. */
void Renderer::append_str(const char* s, size_t n) {
  while (n > 0) {
    size_t space = (BufSize - FlushThreshold) - len_;
    size_t chunk = n < space ? n : space;
    for (size_t i = 0; i < chunk; ++i)
      buf_[len_++] = s[i];
    s += chunk;
    n -= chunk;
    if (len_ >= BufSize - FlushThreshold)
      flush();
  }
}

/** @brief UTF-8 encode a grapheme cluster and append the resulting bytes. */
void Renderer::append_cluster(const Cell& cell) {
  char buf[48];
  size_t n = grapheme::cluster_encode(cell, buf, sizeof(buf));
  if (n > 0)
    append_str(buf, n);
}

/** @brief Emit ANSI CUP "\e[row;colH" (1-based row and column). */
void Renderer::append_cursor(Coord pos) {
  char tmp[32];
  int n =
      std::snprintf(tmp, sizeof(tmp), ANSI_CURSOR_FMT, pos.y + 1, pos.x + 1);
  if (n > 0)
    append_str(tmp, static_cast<size_t>(n));
}

/** @brief Emit ANSI SGR colour codes "\e[38;5;fg;48;5;bgm". */
void Renderer::append_color(Color fg, Color bg) {
  char tmp[32];
  int n = std::snprintf(tmp, sizeof(tmp), ANSI_COLOR_FMT, static_cast<int>(fg),
                        static_cast<int>(bg));
  if (n > 0)
    append_str(tmp, static_cast<size_t>(n));
}

/**
 * @brief Diff a ScreenBuffer against the front buffer and emit ANSI escapes
 *        for changed cells only.
 *
 * Suppresses redundant cursor-positioning and colour codes.  After the pass
 * the front buffer is synced and all dirty flags cleared.
 *
 * Each cell is a complete grapheme cluster (base + optional combining marks,
 * ZWJ sequences, variation selectors, etc.) and is emitted as a single unit.
 * Cells marked with the trail flag (belonging to a preceding wide character)
 * and control characters are skipped.
 *
 * @param buffer The ScreenBuffer to render.
 * @return Ok or Err with Errc::IOError.
 */
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
      const Cell& front = buffer.front(cursor);

      if (front == back)
        continue;

      // Skip cells that are trail halves of wide characters.
      if (back.trail)
        continue;

      // Skip control characters (C0 except NUL, C1, DEL).
      if ((back.base > 0 && back.base < 0x20) ||
          (back.base >= 0x7F && back.base < 0xA0))
        continue;

      int w = back.width > 0 ? back.width : 1;

      if (would_overflow(32))
        if (auto res = flush(); !res)
          return res;

      if (last_cursor_ == InvalidCursor || x != last_cursor_.x + 1 ||
          y != last_cursor_.y)
        append_cursor(cursor);

      if (back.fg != last_fg_ || back.bg != last_bg_) {
        append_color(back.fg, back.bg);
        last_fg_ = back.fg;
        last_bg_ = back.bg;
      }

      append_cluster(back);

      if (w > 1)
        last_cursor_ = {static_cast<uint16_t>(cursor.x + w - 1), cursor.y};
      else
        last_cursor_ = cursor;
    }
  }

  if (auto res = flush(); !res)
    return res;

  buffer.clear_dirty();
  buffer.sync_front();
  last_cursor_ = InvalidCursor;

  return {};
}

}  // namespace vtui
