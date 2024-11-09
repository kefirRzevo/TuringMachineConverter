#include "MiniPrograms.hpp"
#include "TagSystem.hpp"

auto main(int argc, const char* argv[]) -> int {
  using TS = machines::ts::TagSystem;
  using EX = machines::MachineExecutor<TS>;

  try {
    EX e{"Tag System"};
    e.run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
