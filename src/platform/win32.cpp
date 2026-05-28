#include <windows.h>

#include "pal.h"

static DWORD original_input_mode = 0;
static DWORD original_output_mode = 0;

static HANDLE hIn = INVALID_HANDLE_VALUE;
static HANDLE hOut = INVALID_HANDLE_VALUE;

bool vtui_pal_init(void) {
  hIn = GetStdHandle(STD_INPUT_HANDLE);
  hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE)
    return false;

  if (!SetConsoleOutputCP(CP_UTF8))
    return false;

  if (!GetConsoleMode(hOut, &original_output_mode))
    return false;

  if (!GetConsoleMode(hIn, &original_input_mode))
    return false;

  DWORD new_input_mode =
      original_input_mode | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;

  new_input_mode &= ~ENABLE_QUICK_EDIT_MODE;
  new_input_mode &= ~ENABLE_INSERT_MODE;

  if (!SetConsoleMode(hIn, new_input_mode))
    return false;

  DWORD new_output_mode = original_output_mode |
                          ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                          DISABLE_NEWLINE_AUTO_RETURN;

  if (!SetConsoleMode(hOut, new_output_mode))
    return false;

  if (!vtui_pal_write_output((const uint8_t*)"\x1b[?1049h\x1b[?25l", 14))
    return false;

  return true;
}

bool vtui_pal_shutdown(void) {
  bool write_ok =
      vtui_pal_write_output((const uint8_t*)"\x1b[?1049l\x1b[?25h", 14);

  if (!SetConsoleMode(hIn, original_input_mode))
    return false;

  if (!SetConsoleMode(hOut, original_output_mode))
    return false;

  return write_ok;
}

bool vtui_pal_write_output(const uint8_t* buffer, int bytes) {
  DWORD written;
  if (!WriteFile(hOut, buffer, bytes, &written, NULL))
    return false;

  return bytes == written;
}

bool vtui_pal_poll_raw_event(void* out_record) {
  DWORD events_count = 0;
  GetNumberOfConsoleInputEvents(hIn, &events_count);
  if (events_count == 0)
    return false;

  DWORD read;
  if (!ReadConsoleInput(hIn, (INPUT_RECORD*)out_record, 1, &read))
    return false;

  return read == 1;
}