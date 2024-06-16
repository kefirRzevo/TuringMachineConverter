#include "MiniPrograms.hpp"
#include "TuringMachine.hpp"

auto main(int argc, const char* argv[]) -> int {
  using TM = machines::tm::TuringMachine;
  using EX = machines::MachineExecutor<TM>;

  try {
    EX e{"Turing Machine"};
    e.run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
