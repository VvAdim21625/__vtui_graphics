/**
 * @file wcwidth.hpp
 * @brief Terminal cell width of Unicode codepoints (0, 1, or 2 columns).
 */
#pragma once

namespace vtui::utf {

/// Return the number of terminal columns occupied by codepoint @p cp:
///   0  for combining/zero-width characters,
///   1  for normal characters,
///   2  for East Asian wide / fullwidth characters,
///  -1  for C0/C1 control characters (except NUL).
int cell_width(char32_t cp);

}  // namespace vtui::utf
