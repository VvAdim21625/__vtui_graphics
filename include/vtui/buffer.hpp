#pragma once
#include <cstdlib>
#include <vector>

#include "types.hpp"

namespace vtui {

/**
 * @brief Double-buffered screen buffer for terminal output.
 *
 * Maintains a front buffer (currently displayed) and a back buffer (being
 * drawn to). Dirty tracking enables efficient incremental screen updates:
 * only cells modified since the last swap are reported as dirty.
 */
class ScreenBuffer {
public:
  /// Default constructor — buffer is empty (zero size).  Call resize() before
  /// use.
  ScreenBuffer() = default;

  /**
   * @brief Construct a buffer with the given dimensions.
   * @param size Width and height in cells.
   */
  explicit ScreenBuffer(Size size);

  /**
   * @brief Resize the buffer, discarding all content.
   *
   * Both front and back buffers are reset to empty cells and marked fully
   * dirty. If the new cell count is <= the old count, vectors are shrunk
   * rather than reallocated.
   */
  void resize(Size new_size);

  /// Current dimensions of the buffer.
  Size size() const { return size_; }

  /// True if either dimension is zero.
  bool empty() const { return size_.empty(); }

  /**
   * @brief Mutable access to a cell in the back buffer.
   *
   * Marks the cell dirty so it will be flushed on next swap.
   */
  Cell& at(Coord pos);

  /// Read-only access to a cell in the back buffer.
  const Cell& at(Coord pos) const;

  /// Mutable access to a cell in the front (committed) buffer.
  Cell& front(Coord pos);

  /// Read-only access to a cell in the front (committed) buffer.
  const Cell& front(Coord pos) const;

  /// Raw pointer to the back buffer data.
  Cell* data() { return back_.data(); }

  /// Total number of cells in the buffer (width * height).
  size_t cell_count() const { return back_.size(); }

  /**
   * @brief Write a cell with full width-awareness.
   *
   * For wide characters (width > 1, e.g. CJK), trailing cells are
   * automatically marked with the trail flag so the renderer can skip
   * them.  Use this instead of at() when writing non-ASCII codepoints.
   */
  void set_cell(Coord pos, Cell cell);

  /**
   * @brief Fill a rectangular region with a given cell in the back buffer.
   *
   * Cells inside the region are marked dirty. The region is clipped to the
   * buffer bounds.
   */
  void fill(Rect region, Cell cell);

  /**
   * @brief Clear the entire back buffer to a given cell.
   *
   * All cells are marked dirty.
   */
  void clear(Cell cell = Cell{});

  /// Whether the cell at `index` was modified since the last dirty flush.
  bool is_dirty(size_t index) const { return dirty_[index] != 0; }

  /**
   * @brief Mark all cells in a rectangular region as dirty.
   *
   * The region is clipped to the buffer bounds.
   */
  void mark_dirty(Rect region);

  /// Mark every cell as dirty.
  void mark_all_dirty();

  /// Clear all dirty flags (typically done after flushing updates).
  void clear_dirty();

  /// Bulk-sync the front buffer with the back buffer (one memcpy).
  void sync_front();

private:
  Size size_;
  std::vector<Cell> front_;
  std::vector<Cell> back_;
  std::vector<uint8_t> dirty_;

  /// Convert a 2D coordinate to a linear index.
  size_t index_of(Coord pos) const;
};

}  // namespace vtui
