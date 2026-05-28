#pragma once
#include <cstdlib>
#include <vector>

#include "types.hpp"

namespace vtui {

class ScreenBuffer {
public:
  ScreenBuffer() = default;
  explicit ScreenBuffer(Size size);

  void resize(Size new_size);
  Size size() const { return size_; }
  bool empty() const { return size_.empty(); }

  Cell& at(Coord pos);
  const Cell& at(Coord pos) const;

  Cell& front(Coord pos);
  const Cell& front(Coord pos) const;

  Cell* data() { return back_.data(); }
  size_t cell_count() const { return back_.size(); }

  void fill(Rect region, Cell cell);
  void clear(Cell cell = Cell{});

  bool is_dirty(size_t index) const { return dirty_[index]; }
  void mark_dirty(Rect region);
  void mark_all_dirty();
  void clear_dirty();

  void swap();

private:
  Size size_;
  std::vector<Cell> front_;
  std::vector<Cell> back_;
  std::vector<bool> dirty_;

  size_t index_of(Coord pos) const;
};

}  // namespace vtui
