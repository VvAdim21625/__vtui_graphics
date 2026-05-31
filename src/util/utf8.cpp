#include "utf8.hpp"

#include <cstring>

namespace vtui::utf {

/// Count codepoints in a null-terminated UTF-8 string (returns 0 for nullptr).
size_t strlen(const char* s) {
  if (s == nullptr)
    return 0;
  auto p = reinterpret_cast<const unsigned char*>(s);
  size_t count = 0;

  // Unroll loop to reduce branch overhead on long strings.
  while (*p) {
    unsigned char c = p[0];
    if (c == 0)
      break;
    if ((c & 0xC0) != 0x80)
      count++;
    p++;

    c = p[0];
    if (c == 0)
      break;
    if ((c & 0xC0) != 0x80)
      count++;
    p++;

    c = p[0];
    if (c == 0)
      break;
    if ((c & 0xC0) != 0x80)
      count++;
    p++;

    c = p[0];
    if (c == 0)
      break;
    if ((c & 0xC0) != 0x80)
      count++;
    p++;
  }

  return count;
}

/// Number of UTF-8 code units needed to encode a codepoint (1–4).
size_t glyph_len(Codepoint cp) {
  if (cp < 0x80)
    return 1;
  if (cp < 0x800)
    return 2;
  if (cp >= 0xD800 && cp <= 0xDFFF)
    return 0;
  if (cp > 0x10FFFF)
    return 0;
  if (cp < 0x10000)
    return 3;
  return 4;
}

/// Allocate and fill a string with repeated UTF-8 codepoints.
char* repeat(Codepoint cp, size_t count) {
  size_t gl = glyph_len(cp);
  if (count > (static_cast<size_t>(-1) - 1) / gl)
    return nullptr;
  size_t buf_len = count * gl + 1;
  auto* buf = new char[buf_len];

  char glyph[5];
  encode(cp, glyph);

  char* p = buf;
  for (size_t i = 0; i < count; ++i) {
    memcpy(p, glyph, gl);
    p += gl;
  }
  *p = '\0';
  return buf;
}

/// Decode the next codepoint from a UTF-8 string.
const char* next(const char* utf8, Codepoint* codepoint) {
  if (utf8 == nullptr || *utf8 == '\0') {
    if (codepoint)
      *codepoint = 0;
    return nullptr;
  }

  auto p = reinterpret_cast<const unsigned char*>(utf8);
  unsigned char c = p[0];

  if (c < 0x80) {
    if (codepoint)
      *codepoint = static_cast<Codepoint>(c);
    return reinterpret_cast<const char*>(p + 1);
  }

  if ((c & 0xE0) == 0xC0) {
    if (p[1] == '\0') {
      if (codepoint)
        *codepoint = 0;
      return nullptr;
    }
    if (codepoint)
      *codepoint = static_cast<Codepoint>(((c & 0x1F) << 6) | (p[1] & 0x3F));
    return reinterpret_cast<const char*>(p + 2);
  }

  if ((c & 0xF0) == 0xE0) {
    if (p[1] == '\0' || p[2] == '\0') {
      if (codepoint)
        *codepoint = 0;
      return nullptr;
    }
    if (codepoint)
      *codepoint = static_cast<Codepoint>(((uint32_t)(c & 0x0F) << 12) |
                                          ((uint32_t)(p[1] & 0x3F) << 6) |
                                          (uint32_t)(p[2] & 0x3F));
    return reinterpret_cast<const char*>(p + 3);
  }

  if ((c & 0xF8) == 0xF0) {
    if (p[1] == '\0' || p[2] == '\0' || p[3] == '\0') {
      if (codepoint)
        *codepoint = 0;
      return nullptr;
    }
    if (codepoint)
      *codepoint = static_cast<Codepoint>(
          ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12) |
          ((uint32_t)(p[2] & 0x3F) << 6) | (uint32_t)(p[3] & 0x3F));
    return reinterpret_cast<const char*>(p + 4);
  }

  if (codepoint)
    *codepoint = static_cast<Codepoint>(c);
  return reinterpret_cast<const char*>(p + 1);
}

/// Encode a codepoint into a null-terminated UTF-8 sequence (max 5 bytes).
void encode(Codepoint cp, char out_str[5]) {
  if ((cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF) {
    out_str[0] = '\0';
    return;
  }

  auto p = reinterpret_cast<unsigned char*>(out_str);

  if (cp < 0x80) {
    p[0] = static_cast<unsigned char>(cp);
    p[1] = '\0';
    return;
  }

  if (cp < 0x800) {
    p[0] = static_cast<unsigned char>((cp >> 6) | 0xC0);
    p[1] = static_cast<unsigned char>((cp & 0x3F) | 0x80);
    p[2] = '\0';
    return;
  }

  if (cp < 0x10000) {
    p[0] = static_cast<unsigned char>((cp >> 12) | 0xE0);
    p[1] = static_cast<unsigned char>(((cp >> 6) & 0x3F) | 0x80);
    p[2] = static_cast<unsigned char>((cp & 0x3F) | 0x80);
    p[3] = '\0';
    return;
  }

  // 4-byte sequence
  p[0] = static_cast<unsigned char>((cp >> 18) | 0xF0);
  p[1] = static_cast<unsigned char>(((cp >> 12) & 0x3F) | 0x80);
  p[2] = static_cast<unsigned char>(((cp >> 6) & 0x3F) | 0x80);
  p[3] = static_cast<unsigned char>((cp & 0x3F) | 0x80);
  p[4] = '\0';
}

}  // namespace vtui::utf
