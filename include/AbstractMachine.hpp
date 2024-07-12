#pragma once

#include <iostream>

namespace machines {

// template<typename Impl>
// concept bool hasRead = requires (Impl impl) {
//   { impl.read(std::istream&) } -> void;
// };

template<typename Impl>
class Machine {
  Impl* impl() {
    return static_cast<Impl*>(this);
  }

public:
  void read(std::istream& is) {
    return impl()->read(is);
  }

  void dumpTable(std::ostream& os) const {
    return impl()->dumpTable(os);
  }

  void dumpState(std::ostream& os) const {
    return impl()->dumpState(os);
  }

  void step() {
    return impl()->step();
  }

  bool hlt() const {
    return impl()->hlt();
  }
};

template <typename Impl, typename Machine>
class Converter {
  Impl* impl() {
    return static_cast<Impl*>(this);
  }

public:
  void convert(const Machine& machine, std::ostream& os) {
    return impl()->covert(machine, os);
  }
};

} // namespace machines {
