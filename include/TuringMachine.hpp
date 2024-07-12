#pragma once

#include <deque>
#include <array>
#include <vector>
#include <utility>
#include <algorithm>

#include "Utils.hpp"
#include "AbstractMachine.hpp"

namespace machines {

template <typename Impl>
class Machine;

class TMConverter;

namespace detail {

using utils::checkPattern;
using utils::readLineToSS;
using utils::toBool;
using utils::toChar;

class States {
public:
  using Symbol = bool;
  using StateVal = unsigned int;

  enum class Move : bool {
    L,
    R,
  };

  struct Jump {
    Move move_;
    StateVal newState_;
    Symbol newSym_;
  };

  struct State {
    StateVal state_;
    std::array<Jump, 2U> jumps_;
  };

private:
  using StatePair = std::pair<std::string, State>;

  std::vector<StatePair> states_;
  StateVal halt_;

  auto findState(std::string_view stateStr) {
      auto found = std::find_if(states_.begin(), states_.end(),
        [&](auto&& statePair) { return statePair.first == stateStr; }
      );
      if (found == states_.end()) {
        auto msg = "Can't find state '" + std::string(stateStr) + "'";
        throw std::runtime_error(msg);
      }
      return found;
  }

  auto findState(std::string_view stateStr) const {
      auto found = std::find_if(states_.begin(), states_.end(),
        [&](auto&& statePair) { return statePair.first == stateStr; }
      );
      if (found == states_.end()) {
        auto msg = "Can't find state '" + std::string(stateStr) + "'";
        throw std::runtime_error(msg);
      }
      return found;
  }

  Move getMove(std::string_view string) {
    if (string == "L") {
      return Move::L;
    } else if (string == "R") {
      return Move::R;
    } else {
      auto msg = "Unknown move '" + std::string(string) + "'";
      throw std::runtime_error(msg);
    }
  }

  void readStates(std::istream& is) {
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

  void readHalt(std::istream& is) {
    std::string pat, haltStr;
    is >> pat;
    checkPattern(pat, "halt:");
    std::getline(is, pat);
    is >> haltStr;
    halt_ = getState(haltStr).state_;
  }

  void readTable(std::istream& is) {
    std::string pat, stateStr, nextStr, moveStr;
    is >> pat;
    checkPattern(pat, "table:");
    std::getline(is, pat);
    Symbol sym, write;
    for (int i = 0; i != 2 * (states_.size() - 1); ++i) {
      auto iss = readLineToSS(is);
      iss >> stateStr >> sym >> write >> nextStr >> moveStr;
      auto& state = getState(stateStr);
      auto& next = getState(nextStr);
      auto& jump = state.jumps_[sym];
      jump.move_ = getMove(moveStr);
      jump.newState_ = next.state_;
      jump.newSym_ = write;
    }
  }

  void dumpStates(std::ostream& os) const {
    os << "states:\n";
    for (auto && state : states_) {
      os << state.first << "\t";
    }
    os << "\n\n";
  }

  void dumpHalt(std::ostream& os) const {
    os << "halt:\n";
    os << getStateStr(halt_) << "\n\n";
  }

  void dumpTable(std::ostream& os) const {
    os << "table:\n";
    for (auto&& [stateStr, state] : states_) {
      if (hlt(state.state_)) {
        continue;
      }
      for (auto i = 0; i != state.jumps_.size(); ++i) {
        auto& jump = state.jumps_[i];
        os << stateStr << "\t" << i << "\t" << jump.newSym_ << "\t";
        os << getStateStr(jump.newState_) << "\t";
        os << (jump.move_ == Move::L ? "L" : "R") << "\n";
      }
    }
    os << "\n\n";
  }

  friend class ::machines::TMConverter;

  public:
    void read(std::istream& is) {
      readStates(is);
      readHalt(is);
      readTable(is);
    }

    std::string_view getStateStr(StateVal val) const {
      return states_.at(val).first;
    }

    State& getState(StateVal val) {
      return states_.at(val).second;
    }

    const State& getState(StateVal val) const {
      return states_.at(val).second;
    }

    State& getState(std::string_view stateStr) {
      auto found = findState(stateStr);
      return found->second;
    }

    const State& getState(std::string_view stateStr) const {
      auto found = findState(stateStr);
      return found->second;
    }

    void dump(std::ostream& os) const {
      dumpStates(os);
      dumpHalt(os);
      dumpTable(os);
    }

    bool hlt(StateVal state) const {
      return state == halt_;
    }
};

struct Tape {
  using Symbol = States::Symbol;
  using StateVal = States::StateVal;

  StateVal state_;
  Symbol head_;
  std::deque<Symbol> left_;
  std::deque<Symbol> right_;

  void read(std::istream& is, const States& states) {
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
};

} // namespace detail {

class TuringMachine :  public Machine<TuringMachine> {
  using States = detail::States;
  using Tape = detail::Tape;
  using Symbol = States::Symbol;
  using StateVal = States::StateVal;

  States states_;
  Tape tape_;

  friend class ::machines::TMConverter;

public:
  void read(std::istream& is) {
    states_.read(is);
    tape_.read(is, states_);
  }

  void dumpTable(std::ostream& os) const {
    states_.dump(os);
  }

  void dumpState(std::ostream& os) const {
    tape_.dump(os, states_);
  }

  void step() {
    auto state = states_.getState(tape_.state_);
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

  bool hlt() const {
    return states_.hlt(tape_.state_);
  }
};

class TMConverter : public Converter<TMConverter, TuringMachine> {
  using Symbol = TuringMachine::Symbol;
  using StateVal = TuringMachine::StateVal;
  using States = detail::States;
  using State = States::State;
  using Move = States::Move;

  enum class TagType {
    Hk0,
    Hk1,
    Hk,
    Lk0,
    Lk1,
    Lk,
    Rk0,
    Rk1,
    Rk,
    Rkk,
  };

  std::string getTag(std::string_view stateStr, TagType type) const {
    std::string state {stateStr};
    switch (type) {
    case TagType::Hk0:
      return "H" + state + "0";
    case TagType::Hk1:
      return "H" + state + "1";
    case TagType::Hk:
      return "H" + state;
    case TagType::Lk0:
      return "L" + state + "0";
    case TagType::Lk1:
      return "L" + state + "1";
    case TagType::Lk:
      return "L" + state;
    case TagType::Rk0:
      return "R" + state + "0";
    case TagType::Rk1:
      return "R" + state + "1";
    case TagType::Rk:
      return "R" + state;
    case TagType::Rkk:
      return "R" + state + state;
    default:
      auto msg = "Unreachable";
      throw std::runtime_error(msg);
    }
  }

  std::vector<std::string> getAppends(const States& states, const State& state,
        std::string_view stateStr, TagType type) const {
    std::vector<std::string> appends;
    auto i = 0;
    switch (type) {
    case TagType::Hk1:
    case TagType::Lk1:
    case TagType::Rk1:
      i = 1;
      break;
    default:
      break;
    }
    auto jump = state.jumps_[i];
    auto write = jump.newSym_;
    auto move = jump.move_;
    auto newStateStr = states.getStateStr(jump.newState_);
    auto a = write == true && move == Move::L;
    auto b = i == 0;
    auto c = write == true && move == Move::R;
    auto d = move == Move::R;
    auto e = move == Move::L;
    switch (type) {
    case TagType::Hk0:
    case TagType::Hk1: {
      auto tagHk = getTag(newStateStr, TagType::Hk);
      auto tagLk = getTag(newStateStr, TagType::Lk);
      auto tagRkk = getTag(newStateStr, TagType::Rkk);
      if (a) {
        appends.insert(appends.end(), 2, tagRkk);
      }
      if (b) {
        appends.emplace_back(tagHk);
      }
      appends.emplace_back(tagHk);
      if (c) {
        appends.insert(appends.end(), 2, tagLk);
      }
      break;
    }
    case TagType::Hk: {
      auto tag1 = getTag(stateStr, TagType::Hk1);
      auto tag0 = getTag(stateStr, TagType::Hk0);
      appends.emplace_back(tag1);
      appends.emplace_back(tag0);
      break;
    }
    case TagType::Lk0:
    case TagType::Lk1: {
      auto tagLk = getTag(newStateStr, TagType::Lk);
      if (d) {
        appends.insert(appends.end(), 3, tagLk);
      }
      appends.emplace_back(tagLk);
      break;
    }
    case TagType::Lk: {
      auto tag1 = getTag(stateStr, TagType::Lk1);
      auto tag0 = getTag(stateStr, TagType::Lk0);
      appends.emplace_back(tag1);
      appends.emplace_back(tag0);
      break;
    }
    case TagType::Rk0:
    case TagType::Rk1: {
      auto tagRk = getTag(newStateStr, TagType::Rk);
      appends.emplace_back(tagRk);
      if (e) {
        appends.insert(appends.end(), 3, tagRk);
      }
      break;
    }
    case TagType::Rk: {
      auto tag1 = getTag(stateStr, TagType::Rk1);
      auto tag0 = getTag(stateStr, TagType::Rk0);
      appends.emplace_back(tag1);
      appends.emplace_back(tag0);
      break;
    }
    case TagType::Rkk: {
      auto tag = getTag(stateStr, TagType::Rk);
      appends.insert(appends.end(), 2, tag);
      break;
    }
    default:
      auto msg = "Unreachable";
      throw std::runtime_error(msg);
    }
    return appends;
  }

  size_t toNumber(const std::deque<bool> &queue) const {
    size_t num = 0;
    for (auto i = 0; i != queue.size(); ++i) {
      if (queue[i]) {
        num += 1 << (queue.size() - i - 1);
      }
    }
    return num;
  }

  void writeStates(const TuringMachine& tm, std::ostream& os) const {
    auto&& states = tm.states_.states_;
    utils::EnumRange<TagType> range{TagType::Hk0, TagType::Rkk};
    os << "tags:\n";
    for (auto&& [stateStr, state] : states) {
      for (auto&& tagType : range) {
        os << getTag(stateStr, tagType) << " ";
      }
    }
    os << "\n\n";
  }

  void writeHalt(const TuringMachine& tm, std::ostream& os) const {
    auto&& states = tm.states_;
    auto&& haltStr = states.getStateStr(states.halt_);
    auto tagHk0 = getTag(haltStr, TagType::Hk0);
    auto tagHk1 = getTag(haltStr, TagType::Hk1);
    os << "halt:\n";
    os << tagHk0 << " " << tagHk1 << " ";
    os << "\n\n";
  }

  void writeTable(const TuringMachine& tm, std::ostream& os) const {
    auto&& states = tm.states_.states_;
    utils::EnumRange<TagType> range{TagType::Hk0, TagType::Rkk};
    os << "table:\n";
    for (auto&& [stateStr, state] : states) {
      for (auto&& tagType : range) {
        auto appends = getAppends(tm.states_, state, stateStr, tagType);
        os << getTag(stateStr, tagType) << " -> ";
        for (auto&& string : appends) {
          os << string << " ";
        }
        os << "\n";
      }
    }
    os << "\n\n";
  }

  void writeInitial(const TuringMachine& tm, std::ostream& os) const {
    auto& states = tm.states_;
    auto& tape = tm.tape_;
    auto leftNum = toNumber(tape.left_), rightNum = toNumber(tape.right_);
    auto stateStr = states.getStateStr(tape.state_);
    auto tagHk0 = getTag(stateStr, TagType::Hk0);
    auto tagHk1 = getTag(stateStr, TagType::Hk1);
    auto tagLk0 = getTag(stateStr, TagType::Lk0);
    auto tagLk1 = getTag(stateStr, TagType::Lk1);
    auto tagRk0 = getTag(stateStr, TagType::Rk0);
    auto tagRk1 = getTag(stateStr, TagType::Rk1);
    std::vector<std::string> appends;
    os << "initial:\n";
    if (tape.head_) {
      appends.emplace_back(tagHk1);
      appends.emplace_back(tagHk0);
      for (auto i = 0; i != leftNum; ++i) {
        appends.emplace_back(tagLk1);
        appends.emplace_back(tagLk0);
      }
      for (auto i = 0; i != rightNum; ++i) {
        appends.emplace_back(tagRk1);
        appends.emplace_back(tagRk0);
      }
    } else {
      appends.emplace_back(tagHk0);
      appends.emplace_back(tagHk1);
      for (auto i = 0; i != leftNum; ++i) {
        appends.emplace_back(tagLk0);
        appends.emplace_back(tagLk1);
      }
      for (auto i = 0; i != rightNum; ++i) {
        appends.emplace_back(tagRk0);
        appends.emplace_back(tagRk1);
      }
      appends.pop_back();
    }
    for (auto&& tag : appends) {
      os << tag << " ";
    }
    os << "\n";
  }

public:
  void convert(const TuringMachine& tm, std::ostream& os) const {
    writeStates(tm, os);
    writeHalt(tm, os);
    writeTable(tm, os);
    writeInitial(tm, os);
  }
};

} // namespace machines
