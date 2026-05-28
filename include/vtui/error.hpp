#pragma once
#include <type_traits>
#include <utility>

namespace vtui {

/**
 * @brief Error codes returned by Result<T> on failure.
 */
enum class Errc {
  Ok = 0,
  InvalidArgs = -1,
  MemoryAlloc = -2,
  IOError = -3,
  Unknown = -255,
};

struct OkTag {};
struct ErrTag {};

inline constexpr OkTag Ok{};
inline constexpr ErrTag Err{};

namespace detail {

/**
 * @brief Prints a panic message to stderr and terminates the process.
 * @param msg Message to display before aborting.
 */
[[noreturn]] void abort_with(const char* msg);

}  // namespace detail

/*  ========================================================================
    Result<T>
========================================================================
*/

/**
 * @brief Rust-style result type that holds either a value T or an error code.
 *
 * Usage:
 * @code
 *   Result<int> r1{Ok, 42};
 *   Result<int> r2{Err, Errc::InvalidArgs};
 *   r1.unwrap();     // panics if Err
 *   r1.expect("msg");
 *   r1.unwrap_or(0);
 *   r1.map([](int x) { return x * 2; });
 *   r1.and_then([](int x) -> Result<float> { ... });
 * @endcode
 *
 * @tparam T The success value type (must not be void; use Result<void>).
 */
template <typename T>
class Result {
  static_assert(!std::is_same_v<T, void>, "use Result<void> for void");
  static_assert(std::is_destructible_v<T>, "T must be destructible");

  alignas(T) unsigned char storage_[sizeof(T)];
  Errc error_ = Errc::Ok;
  bool ok_ = false;

  T* ptr() { return reinterpret_cast<T*>(storage_); }
  const T* ptr() const { return reinterpret_cast<const T*>(storage_); }

  void abort_with(const char* msg) const { detail::abort_with(msg); }

public:
  Result(OkTag, const T& val) noexcept(std::is_nothrow_copy_constructible_v<T>)
      : error_(Errc::Ok), ok_(true) {
    ::new (storage_) T(val);
  }

  Result(OkTag, T&& val) noexcept(std::is_nothrow_move_constructible_v<T>)
      : error_(Errc::Ok), ok_(true) {
    ::new (storage_) T(std::move(val));
  }

  Result(ErrTag, Errc err) noexcept : error_(err), ok_(false) {}

  ~Result() {
    if (ok_)
      ptr()->~T();
  }

  Result(const Result& other) : error_(other.error_), ok_(other.ok_) {
    if (ok_)
      ::new (storage_) T(*other.ptr());
  }

  Result(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
      : error_(other.error_), ok_(other.ok_) {
    if (ok_)
      ::new (storage_) T(std::move(*other.ptr()));
  }

  Result& operator=(const Result&) = delete;
  Result& operator=(Result&&) = delete;

  /**
   * @brief Returns true if the Result holds a value.
   */
  bool is_ok() const { return ok_; }

  /**
   * @brief Returns true if the Result holds an error.
   */
  bool is_err() const { return !ok_; }

  explicit operator bool() const { return ok_; }

  /**
   * @brief Unwraps the contained value, panicking if this is an error.
   */
  T& unwrap() {
    if (!ok_)
      abort_with("called unwrap() on an Err value");
    return *ptr();
  }

  const T& unwrap() const {
    if (!ok_)
      abort_with("called unwrap() on an Err value");
    return *ptr();
  }

  /**
   * @brief Unwraps with a custom panic message.
   * @param msg Message displayed on panic.
   */
  T& expect(const char* msg) {
    if (!ok_)
      abort_with(msg);
    return *ptr();
  }

  const T& expect(const char* msg) const {
    if (!ok_)
      abort_with(msg);
    return *ptr();
  }

  /**
   * @brief Returns the contained value or a default on error.
   * @param default_val Fallback value.
   */
  T unwrap_or(T default_val) const {
    if (!ok_)
      return default_val;
    return *ptr();
  }

  T& operator*() { return *ptr(); }
  const T& operator*() const { return *ptr(); }

  /**
   * @brief Returns the error code (meaningful only when is_err()).
   */
  Errc error() const { return error_; }

  /**
   * @brief Applies a function to the contained value.
   * @param f Mapping function T -> U.
   */
  template <typename F>
  auto map(F&& f) -> Result<decltype(f(std::declval<T&>()))> {
    using U = decltype(f(std::declval<T&>()));
    if (!ok_)
      return Result<U>{Err, error_};
    return Result<U>{Ok, f(*ptr())};
  }

  template <typename F>
  auto map(F&& f) const -> Result<decltype(f(std::declval<const T&>()))> {
    using U = decltype(f(std::declval<const T&>()));
    if (!ok_)
      return Result<U>{Err, error_};
    return Result<U>{Ok, f(*ptr())};
  }

  /**
   * @brief Chains a function that returns a Result.
   * @param f Function T -> Result<U>.
   */
  template <typename F>
  auto and_then(F&& f) -> decltype(f(std::declval<T&>())) {
    if (!ok_)
      return {Err, error_};
    return f(*ptr());
  }

  template <typename F>
  auto and_then(F&& f) const -> decltype(f(std::declval<const T&>())) {
    if (!ok_)
      return {Err, error_};
    return f(*ptr());
  }
};

/*  ========================================================================
    Result<void>
========================================================================
*/

/**
 * @brief Specialization of Result for void success values.
 *
 * Semantically equivalent to Result<T>, but holds no value on success.
 */
template <>
class Result<void> {
  Errc error_ = Errc::Ok;

  void abort_with(const char* msg) const { detail::abort_with(msg); }

public:
  Result() = default;
  Result(ErrTag, Errc err) noexcept : error_(err) {}

  /**
   * @brief Returns true if the Result is successful.
   */
  bool is_ok() const { return error_ == Errc::Ok; }

  /**
   * @brief Returns true if the Result holds an error.
   */
  bool is_err() const { return error_ != Errc::Ok; }

  explicit operator bool() const { return is_ok(); }

  /**
   * @brief Unwraps, panicking if this is an error.
   */
  void unwrap() const {
    if (is_err())
      abort_with("called unwrap() on an Err value");
  }

  /**
   * @brief Unwraps with a custom panic message.
   * @param msg Message displayed on panic.
   */
  void expect(const char* msg) const {
    if (is_err())
      abort_with(msg);
  }

  /**
   * @brief Returns the error code (meaningful only when is_err()).
   */
  Errc error() const { return error_; }

  /**
   * @brief Chains a function that returns a Result.
   * @param f Function () -> Result<U>.
   */
  template <typename F>
  auto and_then(F&& f) -> decltype(f()) {
    if (is_err())
      return Result<void>{Err, error_};
    return f();
  }

  template <typename F>
  auto and_then(F&& f) const -> decltype(f()) {
    if (is_err())
      return Result<void>{Err, error_};
    return f();
  }
};

}  // namespace vtui
