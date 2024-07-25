#pragma once

#include <algorithm>
#include <array>
#include <deque>
#include <utility>
#include <vector>

#include "AbstractMachine.hpp"
#include "Utils.hpp"

namespace machines {

class TMConverter;

namespace tm {

using utils::checkPattern;
using utils::readLineToSS;

class States final {
public:
  using Symbol = bool;
  using StateVal = unsigned int;

  enum class Move : bool {
    L,
    R,
  };

  struct Jump final {
    Move move_;
    StateVal newState_;
    Symbol newSym_;
  };

  struct State final {
    StateVal state_;
    std::array<Jump, 2U> jumps_;
  };

  using StatePair = std::pair<std::string, State>;

private:
  std::vector<StatePair> states_;
  StateVal halt_;

  auto findState(std::string_view stateStr) {
    auto found =
        std::find_if(states_.begin(), states_.end(), [&](auto &&statePair) {
          return statePair.first == stateStr;
        });
    if (found == states_.end()) {
      auto msg = "Can't find state '" + std::string(stateStr) + "'";
      throw std::runtime_error(msg);
    }
    return found;
  }

  auto findState(std::string_view stateStr) const {
    auto found =
        std::find_if(states_.begin(), states_.end(), [&](auto &&statePair) {
          return statePair.first == stateStr;
        });
    if (found == states_.end()) {
      auto msg = "Can't find state '" + std::string(stateStr) + "'";
      throw std::runtime_error(msg);
    }
    return found;
  }

  static Move getMove(std::string_view string) {
    if (string == "L") {
      return Move::L;
    } else if (string == "R") {
      return Move::R;
    } else {
      auto msg = "Unknown move '" + std::string(string) + "'";
      throw std::runtime_error(msg);
    }
  }

  void readStates(std::istream &is) {
    std::string pat, stateStr;
    is >> pat;
    checkPattern(pat, "states:");
    std::getline(is, pat);
    auto iss = readLineToSS(is);
    while (iss >> stateStr) {
      states_.emplace_back(stateStr, State{});
      states_.back().second.state_ = states_.size() - 1;
    }
    assert(!states_.empty());
  }

  void readHalt(std::istream &is) {
    std::string pat, haltStr;
    is >> pat;
    checkPattern(pat, "halt:");
    std::getline(is, pat);
    is >> haltStr;
    halt_ = getState(haltStr).state_;
  }

  void readTable(std::istream &is) {
    std::string pat, stateStr, nextStr, moveStr;
    is >> pat;
    checkPattern(pat, "table:");
    std::getline(is, pat);
    Symbol sym, write;
    for (int i = 0; i != 2 * (states_.size() - 1); ++i) {
      auto iss = readLineToSS(is);
      iss >> stateStr >> sym >> write >> nextStr >> moveStr;
      auto &state = getState(stateStr);
      auto &next = getState(nextStr);
      auto &jump = state.jumps_[sym];
      jump.move_ = getMove(moveStr);
      jump.newState_ = next.state_;
      jump.newSym_ = write;
    }
  }

  void dumpStates(std::ostream &os) const {
    os << "states:\n";
    for (auto &&state : states_) {
      os << state.first << "\t";
    }
    os << "\n\n";
  }

  void dumpHalt(std::ostream &os) const {
    os << "halt:\n";
    os << getStateStr(halt_) << "\n\n";
  }

  void dumpTable(std::ostream &os) const {
    os << "table:\n";
    for (auto &&[stateStr, state] : states_) {
      if (hlt(state.state_)) {
        continue;
      }
      for (auto i = 0; i != state.jumps_.size(); ++i) {
        auto &jump = state.jumps_[i];
        os << stateStr << "\t" << i << "\t" << jump.newSym_ << "\t";
        os << getStateStr(jump.newState_) << "\t";
        os << (jump.move_ == Move::L ? "L" : "R") << "\n";
      }
    }
    os << "\n\n";
  }

public:
  void read(std::istream &is) {
    readStates(is);
    readHalt(is);
    readTable(is);
  }

  std::string_view getStateStr(StateVal val) const {
    return states_.at(val).first;
  }

  State &getState(StateVal val) { return states_.at(val).second; }

  const State &getState(StateVal val) const { return states_.at(val).second; }

  State &getState(std::string_view stateStr) {
    auto found = findState(stateStr);
    return found->second;
  }

  const State &getState(std::string_view stateStr) const {
    auto found = findState(stateStr);
    return found->second;
  }

  void dump(std::ostream &os) const {
    dumpStates(os);
    dumpHalt(os);
    dumpTable(os);
  }

  bool hlt(StateVal state) const noexcept { return state == halt_; }

  StateVal getHalt() const noexcept { return halt_; }

  auto begin() const noexcept { return states_.begin(); }

  auto end() const noexcept { return states_.end(); }
};

struct Tape final {
  using Symbol = States::Symbol;
  using StateVal = States::StateVal;

  StateVal state_;
  Symbol head_;
  std::deque<Symbol> left_;
  std::deque<Symbol> right_;

  void read(std::istream &is, const States &states) {
    std::string pat, tapeStr;
    is >> pat;
    checkPattern(pat, "initial:");
    std::getline(is, pat);
    is >> tapeStr;
    checkPattern(tapeStr, "[");
    checkPattern(tapeStr, "]");
    auto leftBound = tapeStr.begin() + tapeStr.find('[');
    auto rightBound = tapeStr.begin() + tapeStr.find(']');
    assert(leftBound < rightBound);
    for (auto it = tapeStr.begin(); it != leftBound; ++it) {
      left_.emplace_back(toBool(*it));
    }
    left_.pop_back();
    head_ = toBool(*std::prev(leftBound));
    for (auto it = std::next(rightBound); it != tapeStr.end(); ++it) {
      right_.emplace_back(toBool(*it));
    }
    auto stateStr = std::string(std::next(leftBound), rightBound);
    state_ = states.getState(stateStr).state_;
  }

  void dump(std::ostream &os, const States &states) const {
    for (auto &&sym : left_) {
      os << toChar(sym);
    }
    os << toChar(head_);
    os << '[' << states.getStateStr(state_) << ']';
    for (auto &&sym : right_) {
      os << toChar(sym);
    }
    os << "\n";
  }

private:
  static bool toBool(char sym) {
    switch (sym) {
    case '0':
      return false;
    case '1':
      return true;
    default: {
      auto msg = "Unknown symbol '" + std::string(1, sym) + "'";
      throw std::runtime_error(msg);
    }
    }
  }

  static char toChar(bool sym) { return sym ? '1' : '0'; }
};

class TuringMachine final : public machines::Machine<TuringMachine> {
  using Symbol = States::Symbol;
  using StateVal = States::StateVal;

  States states_;
  Tape tape_;

  void read(std::istream &is) {
    states_.read(is);
    tape_.read(is, states_);
  }

  void dumpTable(std::ostream &os) const { states_.dump(os); }

  void dumpState(std::ostream &os) const { tape_.dump(os, states_); }

  void step() {
    auto& state = states_.getState(tape_.state_);
    auto jump = state.jumps_[tape_.head_];
    switch (jump.move_) {
    case States::Move::L:
      tape_.right_.emplace_front(jump.newSym_);
      if (!tape_.left_.empty()) {
        tape_.head_ = tape_.left_.back();
        tape_.left_.pop_back();
      } else {
        tape_.head_ = false;
      }
      break;
    case States::Move::R:
      tape_.left_.emplace_back(jump.newSym_);
      if (!tape_.right_.empty()) {
        tape_.head_ = tape_.right_.front();
        tape_.right_.pop_front();
      } else {
        tape_.head_ = false;
      }
    }
    tape_.state_ = jump.newState_;
  }

  bool hlt() const noexcept { return states_.hlt(tape_.state_); }

  friend class machines::TMConverter;

public:
  void execute(std::istream &is, std::ostream& os, DumpLvl lvl) {
      read(is);
      dumpTable(os);
      while (!hlt()) {
        if (lvl > 0) {
          dumpState(os);
        }
        step();
      }
      dumpState(os);
      os.flush();
  }
};

} // namespace tm

} // namespace machines
