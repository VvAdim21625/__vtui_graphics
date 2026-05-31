#pragma once
#include <cstddef>

#include "buffer.hpp"
#include "result.hpp"

namespace vtui {

/**
 * @brief Terminal renderer that diffs and flushes a ScreenBuffer via ANSI
 *        escape sequences.
 *
 * On each call to present() the renderer walks the ScreenBuffer row-by-row
 * and emits only the cells that are marked dirty *and* whose back-buffer
 * value differs from the front buffer.  To minimise output size it
 * suppresses redundant cursor-positioning and colour-change ANSI sequences
 * by tracking the last-known cursor position, foreground colour, and
 * background colour.
 *
 * Output is batched into an internal buffer (BufSize bytes) and flushed to
 * the platform output layer (vtui_pal_write_output) only when the buffer
 * nears capacity or at the end of each present() call.
 */
class Renderer {
public:
  /// Construct a renderer and initialise state.
  Renderer();
  /// Destructor — flushes any pending output before destruction.
  ~Renderer();

  /**
   * @brief Diff and flush a screen buffer to the terminal.
   *
   * Iterates every cell; skips non-dirty cells and cells whose back-buffer
   * value matches the front-buffer value.  Emits ANSI cursor-positioning
   * and colour codes as needed, then advances the cursor by writing the
   * cell's UTF-8 codepoint.
   *
   * After the flush the front buffer is updated to match the back buffer
   * and all dirty flags are cleared.
   *
   * @param buffer The ScreenBuffer to render.
   * @return Ok on success, or Err with Errc::IOError on write failure.
   */
  Result<void> present(ScreenBuffer& buffer);

private:
  /// Internal output buffer size in bytes.
  static constexpr size_t BufSize = 4096;

  /// Minimum free space required after a flush is triggered.
  static constexpr size_t FlushThreshold = 64;

  /// Sentinel value indicating no known cursor position.
  static constexpr Coord InvalidCursor{0xFFFF, 0xFFFF};

  char buf_[BufSize];
  size_t len_ = 0;

  Coord last_cursor_{InvalidCursor};
  Color last_fg_{Color::Default};
  Color last_bg_{Color::Default};

  /// Append a single byte to the output buffer, auto-flushing if full.
  void append(char c);

  /// Append a byte span to the output buffer, auto-flushing if near-full.
  void append_str(const char* s, size_t n);

  /// Encode a grapheme cluster as UTF-8 and append it.
  void append_cluster(const Cell& cell);

  /// Emit an ANSI CUP (cursor position) sequence "\e[row;colH".
  void append_cursor(Coord pos);

  /// Emit ANSI SGR foreground/background colour codes "\e[38;5;fg;48;5;bgm".
  void append_color(Color fg, Color bg);

  /// Flush the internal buffer to the platform output layer.
  Result<void> flush();

  /// True if appending `extra` bytes would exceed the flush threshold.
  bool would_overflow(size_t extra) const;
};

}  // namespace vtui
