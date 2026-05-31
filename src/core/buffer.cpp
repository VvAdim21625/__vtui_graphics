#include "vtui/buffer.hpp"

#include <algorithm>
#include <cassert>

namespace vtui {

/** @copydoc ScreenBuffer::ScreenBuffer */
ScreenBuffer::ScreenBuffer(Size size) {
  resize(size);
}

/**
 * @brief Resize to new dimensions.
 *
 * If growing, new cells are default-initialised and marked dirty.
 * If shrinking, excess cells are discarded and all remaining cells are
 * marked dirty.
 */
void ScreenBuffer::resize(Size new_size) {
  if (new_size.empty()) {
    size_ = {};
    front_.clear();
    back_.clear();
    dirty_.clear();
    return;
  }

  size_t old_count = front_.size();
  size_t new_count = static_cast<size_t>(new_size.width) * new_size.height;

  if (new_count > old_count) {
    front_.resize(new_count, Cell{});
    back_.resize(new_count, Cell{});
    dirty_.resize(new_count, true);
  } else {
    front_.resize(new_count);
    back_.resize(new_count);
    dirty_.resize(new_count);
  }

  size_ = new_size;
  mark_all_dirty();
}

/** @brief Linear index from 2D coordinate (row-major). */
size_t ScreenBuffer::index_of(Coord pos) const {
  assert(pos.x < size_.width && pos.y < size_.height);
  return static_cast<size_t>(pos.y) * size_.width + pos.x;
}

/** @brief Back-buffer access — marks the cell dirty for next flush. */
Cell& ScreenBuffer::at(Coord pos) {
  size_t idx = index_of(pos);
  dirty_[idx] = true;
  return back_[idx];
}

/** @brief Back-buffer read-only access (does not mark dirty). */
const Cell& ScreenBuffer::at(Coord pos) const {
  return back_[index_of(pos)];
}

/** @copydoc ScreenBuffer::front */
Cell& ScreenBuffer::front(Coord pos) {
  return front_[index_of(pos)];
}

/** @copydoc ScreenBuffer::front */
const Cell& ScreenBuffer::front(Coord pos) const {
  return front_[index_of(pos)];
}

/** @copydoc ScreenBuffer::set_cell */
void ScreenBuffer::set_cell(Coord pos, Cell cell) {
  cell.compute_width();
  size_t idx = index_of(pos);
  back_[idx] = cell;
  dirty_[idx] = true;
  if (cell.width > 1) {
      for (int i = 1; i < cell.width && pos.x + i < size_.width; ++i) {
          back_[idx + i].trail = 1;
          dirty_[idx + i] = true;
      }
  }
}

/** @brief Fill a rectangular region with a cell value (clipped to bounds).
 *
 * Iterates row-by-row, writing to the back buffer and setting dirty flags
 * for every cell that falls within the (clipped) region.  For wide characters
 * trailing cells are marked with the trail flag.
 */
void ScreenBuffer::fill(Rect region, Cell cell) {
  region = region.intersect({{0, 0}, size_});
  if (region.empty())
    return;

  cell.compute_width();
  Cell trail{};
  trail.trail = 1;
  uint16_t buf_w = size_.width;
  for (uint16_t r = region.pos.y; r < region.pos.y + region.size.height;
       ++r) {
    size_t base = static_cast<size_t>(r) * buf_w;
    for (uint16_t c = region.pos.x; c < region.pos.x + region.size.width;) {
      size_t idx = base + c;
      back_[idx] = cell;
      dirty_[idx] = true;
      for (int i = 1; i < cell.width && c + i < buf_w; ++i) {
        back_[idx + i] = trail;
        dirty_[idx + i] = true;
      }
      c = static_cast<uint16_t>(c + cell.width);
    }
  }
}

/** @brief Fill the entire back buffer with a cell and mark all cells dirty. */
void ScreenBuffer::clear(Cell cell) {
  cell.compute_width();
  if (cell.width > 1) {
    fill({{0, 0}, size_}, cell);
  } else {
    std::fill(back_.begin(), back_.end(), cell);
    mark_all_dirty();
  }
}

/** @brief Mark a rectangular region as dirty without modifying cell contents.
 */
void ScreenBuffer::mark_dirty(Rect region) {
  region = region.intersect({{0, 0}, size_});
  if (region.empty())
    return;

  for (uint16_t r = region.pos.y; r < region.pos.y + region.size.height; ++r) {
    size_t base = static_cast<size_t>(r) * size_.width;
    for (uint16_t c = region.pos.x; c < region.pos.x + region.size.width; ++c)
      dirty_[base + c] = true;
  }
}

/** @copydoc ScreenBuffer::mark_all_dirty */
void ScreenBuffer::mark_all_dirty() {
  std::fill(dirty_.begin(), dirty_.end(), true);
}

/** @copydoc ScreenBuffer::clear_dirty */
void ScreenBuffer::clear_dirty() {
  std::fill(dirty_.begin(), dirty_.end(), false);
}

/** @copydoc ScreenBuffer::sync_front */
void ScreenBuffer::sync_front() {
  std::copy(back_.begin(), back_.end(), front_.begin());
}

}  // namespace vtui
