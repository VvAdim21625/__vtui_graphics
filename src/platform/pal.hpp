/**
 * @file pal.hpp
 * @brief Platform Abstraction Layer — minimal API for console I/O.
 *
 * Each platform (win32, linux, etc.) provides an implementation of these
 * functions.  A single implementation is compiled per target.
 */
#pragma once
#include <cstdint>

namespace vtui::pal {

/// Initialise the console/platform for vtui.  Returns true on success.
bool init();

/// Shut down the console/platform and restore original state.  Returns true on
/// success.
bool shutdown();

/// Read raw input bytes into @p buffer (up to @p max_bytes).  Returns true on
/// success.
bool write_input(uint8_t* buffer, int max_bytes);

/// Write @p bytes from @p buffer to the console output.  Returns true on
/// success.
bool write_output(const uint8_t* buffer, int bytes);

/// Non-blocking poll for a raw input event.  Writes the platform-specific
/// event record to @p out_record.  Returns true if an event was available.
bool poll_raw_event(void* out_record);

}  // namespace vtui::pal
