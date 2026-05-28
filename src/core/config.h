#pragma once

#include "vtui/types.hpp"

#ifndef VTUI_BATCH_BUFFER_SIZE
#define VTUI_BATCH_BUFFER_SIZE 4096
#endif

#define VTUI_FLUSH_THRESHOLD 64
#define VTUI_EST_BYTES_PER_CELL 8

#define VTUI_MAX_UTF8_BYTES 4
#define VTUI_UTF8_BUF_SIZE (VTUI_MAX_UTF8_BYTES + 1)

#define VTUI_MAX_ANSI_SEQ_LEN 32
