#include "vtui/error.hpp"

#include <cstdio>
#include <cstdlib>

namespace vtui::detail {

/// @brief  Prints a panic message to stderr and terminates the process.
/// @param msg  Message to display before aborting.
[[noreturn]] void abort_with(const char* msg) {
  std::fprintf(stderr, "vtui::Result panic: %s\n", msg);
  std::abort();
}

}  // namespace vtui::detail
