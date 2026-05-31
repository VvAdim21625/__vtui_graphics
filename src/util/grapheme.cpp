#include "grapheme.hpp"
#include "utf8.hpp"

namespace vtui::grapheme {

size_t cluster_encode(const Cell& cell, char* out, size_t out_size) {
  if (out_size == 0) return 0;
  size_t pos = 0;
  char tmp[5];

  auto emit = [&](char32_t cp) {
    utf::encode(cp, tmp);
    for (size_t j = 0; tmp[j] && pos < out_size - 1; ++j)
      out[pos++] = tmp[j];
  };

  emit(cell.base);
  for (uint8_t i = 0; i < cell.ext_len; ++i) {
    if (pos >= out_size - 1) break;
    emit(cell.ext[i]);
  }
  out[pos] = '\0';
  return pos;
}

}  // namespace vtui::grapheme
