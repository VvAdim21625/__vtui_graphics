#pragma once
#include <cstddef>
#include "vtui/types.hpp"

namespace vtui::grapheme {

size_t cluster_encode(const Cell& cell, char* out, size_t out_size);

}  // namespace vtui::grapheme
