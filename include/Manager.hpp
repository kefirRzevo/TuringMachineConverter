#pragma once

#include <filesystem>
#include <fstream>

#include "Utils.hpp"

namespace machines {

class Manager {
  std::string_view getProgram(std::string_view str) const {
    return str.substr(0, str.find(' '));
  }

  bool checkProgram(std::string_view prog) const {
    auto path = std::filesystem::path(prog);
    auto name = path.filename().generic_string();
    return name == "ETM" || name == "ETS" || name == "ECTS" || name == "CTM" ||
           name == "CTS";
  }

public:
  void run() {
    for (;;) {
      std::string cmd;
      std::getline(std::cin, cmd);
      if (cmd == "q")
        break;
      auto prog = getProgram(cmd);
      if (!checkProgram(prog)) {
        std::cout << "no such program" << std::endl;
        continue;
      }
      if (std::system(cmd.c_str()))
        std::cout << "error" << std::endl;
      else
        std::cout << "ok" << std::endl;
    }
  }
};

} // namespace machines
