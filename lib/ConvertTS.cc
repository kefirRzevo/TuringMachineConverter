#include "MiniPrograms.hpp"
#include "TagSystemConverter.hpp"

auto main(int argc, const char* argv[]) -> int {
  using CTS = machines::TSConverter;
  using CO = machines::MachineConverter<CTS>;

  try {
    CO c{"Tag System Converter"};
    c.run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
