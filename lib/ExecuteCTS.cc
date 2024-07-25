#include "MiniPrograms.hpp"
#include "CyclicTagSystem.hpp"

auto main(int argc, const char* argv[]) -> int {
  using CTS = machines::cts::CyclicTagSystem;
  using EX = machines::MachineExecutor<CTS>;

  try {
    EX e{"Cyclic Tag System"};
    e.run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
