#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace utils {

inline void checkPattern(std::string_view string, const char *pattern) {
  if (string.find(pattern) == string.npos) {
    auto msg = "Can't find pattern '" + std::string(pattern);
    msg += "' in '" + std::string(string) + "'";
    throw std::runtime_error(msg);
  }
}

inline std::istringstream readLineToSS(std::istream &is) {
  std::string string;
  std::getline(is, string);
  std::istringstream iss(string);
  return iss;
}

template <typename T> class EnumIterator {
public:
  using difference_type = ptrdiff_t;
  using value_type = T;
  using reference = T &;
  using pointer = T *;
  using iterator_category = std::forward_iterator_tag;

  explicit EnumIterator(T value) : value_(value) {}

  T operator*() const { return value_; }

  EnumIterator &operator++() {
    value_ = (T)((int)(value_) + 1);
    return *this;
  }

  EnumIterator operator++(int) {
    EnumIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const EnumIterator &rhs) const {
    return value_ == rhs.value_;
  }

  bool operator!=(const EnumIterator &rhs) const {
    return value_ != rhs.value_;
  }

private:
  T value_;
};

template <typename T> class EnumRange {
public:
  EnumRange(T begin, T end) : begin_(begin), end_(end) {}

  EnumIterator<T> begin() const { return EnumIterator<T>(begin_); }

  EnumIterator<T> end() const { return EnumIterator<T>((T)((int)end_ + 1)); }

private:
  T begin_;
  T end_;
};

} // namespace utils
