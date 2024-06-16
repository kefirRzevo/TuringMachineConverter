#pragma once

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace machines {

namespace po = boost::program_options;

template <typename Machine> class MachineExecutor {
  using opt_desc = po::options_description;
  using vars_map = po::variables_map;
  using cmd_line_parser = po::command_line_parser;
  using DumpLvl = typename Machine::DumpLvl;

  opt_desc cmdline_{"Options"};
  std::string input_;
  std::string output_;
  DumpLvl dumpLvl_ = 0;
  bool help_ = false;

  std::string name_;

  void initProgramOptions() {
    opt_desc generic("Generic options");
    generic.add_options()("help,h", "help message");

    opt_desc config("Configuration");
    std::string inDesc = "Input file for running " + name_;
    std::string outDesc = "Run " + name_ + " with output file";
    config.add_options()("in", po::value<std::string>()->required(),
                         inDesc.c_str())(
        "out", po::value<std::string>()->implicit_value(""),
        outDesc.c_str())("dump", po::value<DumpLvl>()->implicit_value(1),
                         "Dump Level during execution");
    cmdline_.add(generic).add(config);
  }

  void parseProgramOptions(int argc, const char *argv[]) {
    vars_map vm;
    try {
      auto parser = cmd_line_parser(argc, argv);
      po::store(parser.options(cmdline_).run(), vm);
      po::notify(vm);

      if (vm.count("help")) {
        std::cout << cmdline_ << std::endl;
        help_ = true;
        return;
      }
      input_ = vm["in"].as<std::string>();
      std::filesystem::path inputPath = input_;
      if (vm.count("out")) {
        output_ = vm["out"].as<std::string>();
      } else {
        auto extension = inputPath.extension();
        auto clear = inputPath.replace_extension().generic_string();
        output_ = clear.append("_dump").append(extension);
      }
      if (vm.count("dump")) {
        dumpLvl_ = vm["dump"].as<DumpLvl>();
      }
    } catch (const po::required_option &e) {
      if (vm.count("help")) {
        std::cout << cmdline_ << std::endl;
        help_ = true;
        return;
      }
      throw;
    } catch (const po::multiple_occurrences &e) {
      if (vm.count("help")) {
        std::cout << cmdline_ << std::endl;
        help_ = true;
        return;
      }
      throw;
    }
  }

public:
  MachineExecutor(std::string_view name) : name_(name) {}

  void run(int argc, const char *argv[]) {
    initProgramOptions();
    parseProgramOptions(argc, argv);
    if (help_) {
      return;
    }
    std::ifstream is{input_};
    std::ofstream os{output_};
    Machine ex;
    ex.execute(is, os, dumpLvl_);
  }
};

template <typename Converter> class MachineConverter {
  using opt_desc = po::options_description;
  using vars_map = po::variables_map;
  using cmd_line_parser = po::command_line_parser;

  opt_desc cmdline_{"Options"};
  std::string input_;
  std::string output_;
  bool help_ = false;

  std::string name_;

  void initProgramOptions() {
    opt_desc generic("Generic options");
    generic.add_options()("help,h", "help message");

    opt_desc config("Configuration");
    std::string inDesc = "Input file for running " + name_;
    std::string outDesc = "Run " + name_ + " with output file";
    config.add_options()("in", po::value<std::string>()->required(),
                         inDesc.c_str())(
        "out", po::value<std::string>()->implicit_value(""), outDesc.c_str());
    cmdline_.add(generic).add(config);
  }

  void parseProgramOptions(int argc, const char *argv[]) {
    vars_map vm;
    try {
      auto parser = cmd_line_parser(argc, argv);
      po::store(parser.options(cmdline_).run(), vm);
      po::notify(vm);

      if (vm.count("help")) {
        std::cout << cmdline_ << std::endl;
        help_ = true;
        return;
      }
      input_ = vm["in"].as<std::string>();
      std::filesystem::path inputPath = input_;
      if (vm.count("out")) {
        output_ = vm["out"].as<std::string>();
      } else {
        auto extension = inputPath.extension();
        auto clear = inputPath.replace_extension().generic_string();
        output_ = clear.append("_converted").append(extension);
      }
      if (vm.count("out"))
        output_ = vm["out"].as<std::string>();
    } catch (const po::error &) {
      if (vm.count("help")) {
        help_ = true;
        return;
      }
      throw;
    }
  }

public:
  MachineConverter(std::string_view name) : name_(name) {}

  void run(int argc, const char *argv[]) {
    initProgramOptions();
    parseProgramOptions(argc, argv);
    if (help_) {
      std::cout << cmdline_ << std::endl;
      return;
    }
    std::ifstream is{input_};
    std::ofstream os{output_};
    Converter c;
    c.convert(is, os);
  }
};

} // namespace machines
