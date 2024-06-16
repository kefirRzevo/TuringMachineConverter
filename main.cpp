#include "include/Machines.hpp"

using namespace machines;

auto main() -> int {
  try {
    std::ifstream is{"res/machine.txt"};
    std::ofstream os{"res/emulation.txt"};
    auto states = States::read(is);
    auto tape = Tape::read(is, states);
    TuringMachine m{states, tape};
    os << "Turing machine emulation" << std::endl;
    m.run(os);
    os << std::endl;
    os << "Tag system emulation" << std::endl;
    TagSystem s{states, tape};
    s.run(os);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
