#include "vtui/buffer.hpp"

#include <algorithm>

namespace vtui {

ScreenBuffer::ScreenBuffer(Size size) {
  resize(size);
}

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

size_t ScreenBuffer::index_of(Coord pos) const {
  return static_cast<size_t>(pos.y) * size_.width + pos.x;
}

Cell& ScreenBuffer::at(Coord pos) {
  size_t idx = index_of(pos);
  dirty_[idx] = true;
  return back_[idx];
}

const Cell& ScreenBuffer::at(Coord pos) const {
  return back_[index_of(pos)];
}

Cell& ScreenBuffer::front(Coord pos) {
  return front_[index_of(pos)];
}

const Cell& ScreenBuffer::front(Coord pos) const {
  return front_[index_of(pos)];
}

void ScreenBuffer::fill(Rect region, Cell cell) {
  region = region.intersect({{0, 0}, size_});
  if (region.empty())
    return;

  for (uint16_t r = region.pos.y; r < region.pos.y + region.size.height; ++r) {
    for (uint16_t c = region.pos.x; c < region.pos.x + region.size.width; ++c) {
      size_t idx = static_cast<size_t>(r) * size_.width + c;
      back_[idx] = cell;
      dirty_[idx] = true;
    }
  }
}

void ScreenBuffer::clear(Cell cell) {
  std::fill(back_.begin(), back_.end(), cell);
  mark_all_dirty();
}

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

void ScreenBuffer::mark_all_dirty() {
  std::fill(dirty_.begin(), dirty_.end(), true);
}

void ScreenBuffer::clear_dirty() {
  std::fill(dirty_.begin(), dirty_.end(), false);
}

void ScreenBuffer::swap() {
  front_.swap(back_);
}

}  // namespace vtui
