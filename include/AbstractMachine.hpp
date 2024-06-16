#pragma once

#include <iostream>

namespace machines {

template <typename Impl> class Machine {
  Impl *impl() { return static_cast<Impl *>(this); }

public:
  using DumpLvl = unsigned int;

  void execute(std::istream &is, std::ostream &os, DumpLvl lvl) {
    return impl()->execute(is, os, lvl);
  }
};

template <typename Impl, typename Machine> class Converter {
  Impl *impl() { return static_cast<Impl *>(this); }

public:
  void convert(const Machine &machine, std::ostream &os) {
    return impl()->convert(machine, os);
  }
};

} // namespace machines
