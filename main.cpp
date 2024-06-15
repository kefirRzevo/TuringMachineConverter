#include "include/Machines.hpp"

using namespace machines;

auto main() -> int {
  try {
    std::ifstream is{"res/machine.txt"};
    auto states = States::read(is);
    auto tape = Tape::read(is, states);
    TuringMachine m{states, tape};
    m.run(std::cout);
    TagSystem s{states, tape};
    s.run(std::cout);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
