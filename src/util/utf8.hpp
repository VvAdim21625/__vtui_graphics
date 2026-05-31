/**
 * @file utf8.hpp
 * @brief UTF-8 string utilities (no C++ stdlib dependency).
 */
#pragma once
#include <cstddef>

#include "vtui/types.hpp"

namespace vtui::utf {

/// Count codepoints (not bytes) in a null-terminated UTF-8 string.
size_t strlen(const char* s);

/// Decode the next codepoint from a UTF-8 string, return pointer past it.
const char* next(const char* utf8, Codepoint* codepoint);

/// Encode a codepoint into a null-terminated UTF-8 sequence (max 4 bytes +
/// NUL).
void encode(Codepoint cp, char out_str[5]);

/// Number of UTF-8 code units needed to represent a codepoint (1–4).
size_t glyph_len(Codepoint cp);

/// Allocate and fill a string with `count` repeated UTF-8 codepoints. Caller
/// must delete[] the result.
char* repeat(Codepoint cp, size_t count);

}  // namespace vtui::utf
