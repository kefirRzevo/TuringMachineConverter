#pragma once

#include <fstream>
#include <optional>
#include <filesystem>
#include <boost/program_options.hpp>

#include "TuringMachine.hpp"
#include "TagSystem.hpp"

namespace machines {

namespace po = boost::program_options;

class Manager {
  std::optional<std::string> input_;
  std::optional<std::string> outputTM_;
  std::optional<std::string> outputTS_;
  std::optional<std::string> convertTM_;
  bool dump_ = false;
  bool help_ = false;

  using opt_desc = po::options_description;
  using pos_opt_desc = po::positional_options_description;
  using vars_map = po::variables_map;
  using cmd_line_parser = po::command_line_parser;

  opt_desc cmdline_{"Options"};
  opt_desc visible_{"Allowed"};
  pos_opt_desc pos_;

  void initProgramOptions() {
    opt_desc generic("Generic options");
    generic.add_options()("help,h", "help message");

    opt_desc config("Configuration");
    config.add_options()
    ("rtm", po::value<std::string>(), "Run turing machine with output file")
    ("rts", po::value<std::string>(), "Run tag system with output file")
    ("ctm", po::value<std::string>(), "Convert turing machine with output file")
    ("dump", po::value<bool>()->implicit_value(true), "Dump while executing");

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input", po::value<std::string>()->required(), "input file");

    visible_.add(generic).add(config);
    cmdline_.add(generic).add(config).add(hidden);
    pos_.add("input", 1);
  }

  void parseProgramOptions(int argc, const char* argv[]) {
    vars_map vm;
    try {
      auto parser = cmd_line_parser(argc, argv);
      po::store(parser.options(cmdline_).positional(pos_).run(), vm);
      po::notify(vm);

      if (vm.count("help")) {
        std::cout << visible_ << std::endl;
        help_ = true;
        return;
      }
      if (vm.count("input")) {
        auto input = vm["input"].as<std::string>();
        input_.emplace(input);
      }
      if (vm.count("rtm")) {
        auto outputTM = vm["rtm"].as<std::string>();
        outputTM_.emplace(outputTM);
      }
      if (vm.count("rts")) {
        auto outputTS = vm["rts"].as<std::string>();
        outputTS_.emplace(outputTS);
      }
      if (vm.count("ctm")) {
        auto convertTM = vm["ctm"].as<std::string>();
        convertTM_.emplace(convertTM);
      }
      if (vm.count("dump")) {
        auto dump = vm["dump"].as<bool>();
        dump_ = dump;
      }
    } catch (const po::required_option &e) {
      if (vm.count("help")) {
        std::cout << visible_ << std::endl;
        help_ = true;
        return;
      } else {
        throw;
      }
    }
  }

  void process() {
    if (help_) {
      return;
    }
    auto input = input_.value();
    if (outputTM_.has_value()) {
      std::ifstream is{input};
      std::ofstream os{outputTM_.value()};
      TuringMachine tm;
      tm.read(is);
      tm.dumpTable(os);
      while (!tm.hlt()) {
        if (dump_) {
          tm.dumpState(os);
        }
        tm.step();
      }
      tm.dumpState(os);
      os.flush();
    }
    if (convertTM_.has_value()) {
      std::ifstream is{input};
      TuringMachine tm;
      tm.read(is);
      std::ofstream os{convertTM_.value()};
      TMConverter c;
      c.convert(tm, os);
      os.flush();
    }
    if (outputTS_.has_value()) {
      std::ifstream is{input};
      std::ofstream os{outputTS_.value()};
      TagSystem ts;
      ts.read(is);
      ts.dumpTable(os);
      while (!ts.hlt()) {
        if (dump_ && ts.onHead()) {
          ts.dumpState(os);
        }
        ts.step();
      }
      ts.dumpState(os);
      os.flush();
    }
  }

public:
  void run(int argc, const char *argv[]) {
    initProgramOptions();
    parseProgramOptions(argc, argv);
    process();
  }
};

} // namespace machines
