#include "MiniPrograms.hpp"
#include "TuringMachineConverter.hpp"

auto main(int argc, const char* argv[]) -> int {
  using CTM = machines::TMConverter;
  using CO = machines::MachineConverter<CTM>;

  try {
    CO c{"Turing Machine Converter"};
    c.run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
