#include "include/Converter.hpp"

auto main() -> int {
  try {
    std::ifstream is{"/Users/timurgolubovich/MIPT/TuringMachineConverter/res/machine.txt"};
    std::ofstream os{"res/emulation.txt"};
    TuringMachine m{is};
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
