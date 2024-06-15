#include "include/Machines.hpp"

auto main() -> int {
  try {
    std::ifstream is{"/Users/timurgolubovich/MIPT/TuringMachineConverter/res/machine.txt"};
    auto states = States::read(is);
    auto tape = Tape::read(is, states);
    TuringMachine m{states, tape};
    m.run(std::cout);
    TagSystem s{states, tape};
    s.dump(std::cout);
    s.makeStep();
    s.dump(std::cout);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
