#include <fstream>
#include "Manager.hpp"

using namespace machines;

auto main(int argc, const char* argv[]) -> int {
  try {
    Manager m;
    m.run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
