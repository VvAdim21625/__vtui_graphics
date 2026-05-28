#include "utf8.h"

#include <cstdlib>
#include <cstring>

#include "../core/config.h"

size_t vtui_utf8_strlen(const char* s) {
  if (s == NULL)
    return 0;
  const unsigned char* p = (const unsigned char*)s;
  size_t count = 0;

  /* Unroll loop to reduce branch overhead on long strings. */
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

size_t vtui_utf8_glyph_len(vtui::Codepoint cp) {
  if (cp < 0x80)
    return 1;
  if (cp < 0x800)
    return 2;
  if (cp < 0x10000)
    return 3;
  return 4;
}

char* vtui_utf8_repeat(vtui::Codepoint cp, size_t count) {
  size_t glyph_len = vtui_utf8_glyph_len(cp);
  size_t buf_len = count * glyph_len + 1;
  char* buf = (char*)malloc(buf_len);
  if (!buf)
    return NULL;

  char glyph[VTUI_UTF8_BUF_SIZE];
  vtui_utf8_encode(cp, glyph);

  char* p = buf;
  for (size_t i = 0; i < count; ++i) {
    memcpy(p, glyph, glyph_len);
    p += glyph_len;
  }
  *p = '\0';
  return buf;
}

const char* vtui_utf8_next(const char* utf8, vtui::Codepoint* codepoint) {
  if (utf8 == NULL || *utf8 == '\0') {
    if (codepoint)
      *codepoint = 0;
    return NULL;
  }

  const unsigned char* p = (const unsigned char*)utf8;
  unsigned char c = p[0];

  if (c < 0x80) {
    if (codepoint)
      *codepoint = static_cast<vtui::Codepoint>(c);
    return (const char*)(p + 1);
  }

  if ((c & 0xE0) == 0xC0) {
    if (p[1] == '\0') {
      if (codepoint)
        *codepoint = 0;
      return NULL;
    }
    if (codepoint)
      *codepoint =
          static_cast<vtui::Codepoint>(((c & 0x1F) << 6) | (p[1] & 0x3F));
    return (const char*)(p + 2);
  }

  if ((c & 0xF0) == 0xE0) {
    if (p[1] == '\0' || p[2] == '\0') {
      if (codepoint)
        *codepoint = 0;
      return NULL;
    }
    if (codepoint)
      *codepoint = static_cast<vtui::Codepoint>(((uint32_t)(c & 0x0F) << 12) |
                                                ((uint32_t)(p[1] & 0x3F) << 6) |
                                                (uint32_t)(p[2] & 0x3F));
    return (const char*)(p + 3);
  }

  if ((c & 0xF8) == 0xF0) {
    if (p[1] == '\0' || p[2] == '\0' || p[3] == '\0') {
      if (codepoint)
        *codepoint = 0;
      return NULL;
    }
    if (codepoint)
      *codepoint = static_cast<vtui::Codepoint>(
          ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12) |
          ((uint32_t)(p[2] & 0x3F) << 6) | (uint32_t)(p[3] & 0x3F));
    return (const char*)(p + 4);
  }

  if (codepoint)
    *codepoint = static_cast<vtui::Codepoint>(c);
  return (const char*)(p + 1);
}

void vtui_utf8_encode(vtui::Codepoint cp, char out_str[5]) {
  unsigned char* p = (unsigned char*)out_str;

  if (cp < 0x80) {
    p[0] = (unsigned char)cp;
    p[1] = '\0';
    return;
  }

  if (cp < 0x800) {
    p[0] = (unsigned char)((cp >> 6) | 0xC0);
    p[1] = (unsigned char)((cp & 0x3F) | 0x80);
    p[2] = '\0';
    return;
  }

  if (cp < 0x10000) {
    p[0] = (unsigned char)((cp >> 12) | 0xE0);
    p[1] = (unsigned char)(((cp >> 6) & 0x3F) | 0x80);
    p[2] = (unsigned char)((cp & 0x3F) | 0x80);
    p[3] = '\0';
    return;
  }

  /* 4-byte sequence */
  p[0] = (unsigned char)((cp >> 18) | 0xF0);
  p[1] = (unsigned char)(((cp >> 12) & 0x3F) | 0x80);
  p[2] = (unsigned char)(((cp >> 6) & 0x3F) | 0x80);
  p[3] = (unsigned char)((cp & 0x3F) | 0x80);
  p[4] = '\0';
}
