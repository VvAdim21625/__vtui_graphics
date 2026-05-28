#pragma once
#include <cstddef>
#include <cstdint>

#include "vtui/types.hpp"

size_t vtui_utf8_strlen(const char* s);
const char* vtui_utf8_next(const char* utf8, vtui::Codepoint* codepoint);
void vtui_utf8_encode(vtui::Codepoint cp, char out_str[5]);
size_t vtui_utf8_glyph_len(vtui::Codepoint cp);
char* vtui_utf8_repeat(vtui::Codepoint cp, size_t count);
