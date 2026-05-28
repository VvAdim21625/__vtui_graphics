#pragma once
#include "types.hpp"

namespace vtui {

struct KeyEvent {
  Codepoint codepoint = 0;
  bool is_special = false;
  bool ctrl = false;
  bool alt = false;
  bool shift = false;
};

struct MouseEvent {
  Coord pos;
  enum Action { LmbPressed, RmbPressed, Hover } action = Hover;
};

struct ResizeEvent {
  Size size;
};

struct Event {
  enum Type { Key, Mouse, Resize, None } type = None;
  union {
    KeyEvent key;
    MouseEvent mouse;
    ResizeEvent resize;
  };
};

}  // namespace vtui
