/**
 * @file event.hpp
 * @brief Input event types for terminal interaction.
 */
#pragma once
#include "types.hpp"

namespace vtui {

/** @brief Keyboard input: a Unicode codepoint or a special (non-character) key
 *         with optional modifier flags. */
struct KeyEvent {
  Codepoint codepoint = 0;
  bool is_special = false;
  bool ctrl = false;
  bool alt = false;
  bool shift = false;
};

/** @brief Mouse button press / release / hover at a screen coordinate. */
struct MouseEvent {
  Coord pos;
  enum Action { LmbPressed, RmbPressed, Hover } action = Hover;
};

/** @brief Terminal resize notification with new dimensions. */
struct ResizeEvent {
  Size size;
};

/** @brief Tagged union of all possible input events. */
struct Event {
  enum Type { Key, Mouse, Resize, None } type = None;
  union {
    KeyEvent key;
    MouseEvent mouse;
    ResizeEvent resize;
  } as;
};

}  // namespace vtui
