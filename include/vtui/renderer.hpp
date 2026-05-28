#pragma once
#include <cstddef>

#include "buffer.hpp"
#include "error.hpp"

namespace vtui {

class Renderer {
public:
  Renderer();
  ~Renderer();

  Result<void> present(ScreenBuffer& buffer);

private:
  static constexpr size_t BufSize = 4096;
  static constexpr size_t FlushThreshold = 64;
  static constexpr Coord InvalidCursor{0xFFFF, 0xFFFF};

  char buf_[BufSize];
  size_t len_ = 0;

  Coord last_cursor_{InvalidCursor};
  Color last_fg_{Color::Default};
  Color last_bg_{Color::Default};

  void append(char c);
  void append_str(const char* s, size_t n);
  void append_utf8(Codepoint cp);
  void append_cursor(Coord pos);
  void append_color(Color fg, Color bg);
  Result<void> flush();
  bool would_overflow(size_t extra) const;
};

}  // namespace vtui
