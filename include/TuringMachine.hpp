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

class States final {
public:
  using Symbol = bool;
  using StateIndx = unsigned int;

  enum class Move : bool {
    L,
    R,
  };

  struct Jump final {
    Move move_;
    Symbol newSym_;
    StateIndx newStateIndx_;
  };

  struct State final {
    StateIndx indx_;
    std::string name_;
    std::array<Jump, 2U> jumps_;
  };

private:
  using StateStorage = std::vector<State>;
  using StateConstIter = StateStorage::const_iterator;

  StateStorage states_;
  StateIndx haltStateIndx_;

  StateIndx getStateIndx(std::string_view stateName) const {
    auto found =
        std::find_if(states_.begin(), states_.end(),
                     [&](auto &&state) { return state.name_ == stateName; });
    if (found == states_.end()) {
      auto msg = "Can not find state '" + std::string(stateName) + "'";
      throw std::runtime_error(msg);
    }
    return found->indx_;
  }

  static Move strToMove(std::string_view string) {
    if (string == "L") {
      return Move::L;
    } else if (string == "R") {
      return Move::R;
    } else {
      auto msg = "Unknown move '" + std::string(string) + "'";
      throw std::runtime_error(msg);
    }
  }

  static std::string_view moveToStr(Move move) {
    return move == Move::L ? "L" : "R";
  }

  void readStates(std::istream &is) {
    std::string pat, stateName;
    is >> pat;
    utils::checkPattern(pat, "states:");
    std::getline(is, pat);
    auto iss = utils::readLineToSS(is);
    while (iss >> stateName) {
      states_.emplace_back(State{});
      auto &state = states_.back();
      state.indx_ = states_.size() - 1;
      state.name_ = stateName;
    }
    assert(!states_.empty());
  }

  void readHalt(std::istream &is) {
    std::string pat, haltName;
    is >> pat;
    utils::checkPattern(pat, "halt:");
    std::getline(is, pat);
    is >> haltName;
    haltStateIndx_ = getStateIndx(haltName);
  }

  void readTable(std::istream &is) {
    std::string pat, stateName, nextStateName, moveStr;
    is >> pat;
    utils::checkPattern(pat, "table:");
    std::getline(is, pat);
    Symbol sym, newSym;
    for (size_t i = 0; i != 2 * (states_.size() - 1); ++i) {
      auto iss = utils::readLineToSS(is);
      iss >> stateName >> sym >> newSym >> nextStateName >> moveStr;
      auto stateIndx = getStateIndx(stateName);
      auto &state = states_.at(stateIndx);
      auto &jump = state.jumps_[sym];
      jump.move_ = strToMove(moveStr);
      jump.newStateIndx_ = getStateIndx(nextStateName);
      jump.newSym_ = newSym;
    }
  }

  void dumpStates(std::ostream &os) const {
    os << "states:\n";
    for (auto &&state : states_)
      os << state.name_ << "\t";
    os << "\n\n";
  }

  void dumpHalt(std::ostream &os) const {
    os << "halt:\n";
    os << states_.at(haltStateIndx_).name_ << "\n\n";
  }

  void dumpTable(std::ostream &os) const {
    os << "table:\n";
    for (auto &&state : states_) {
      if (isHlt(state.indx_))
        continue;
      for (size_t i = 0; i != state.jumps_.size(); ++i) {
        const auto &jump = state.jumps_[i];
        const auto &newState = states_.at(jump.newStateIndx_);
        os << state.name_ << "\t" << i << "\t" << jump.newSym_ << "\t";
        os << newState.name_ << "\t" << moveToStr(jump.move_) << "\n";
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

  const State &getState(StateIndx indx) const { return states_.at(indx); }

  const State &getState(std::string_view stateName) const {
    return states_.at(getStateIndx(stateName));
  }

  void dump(std::ostream &os) const {
    dumpStates(os);
    dumpHalt(os);
    dumpTable(os);
  }

  bool isHlt(StateIndx state) const noexcept { return state == haltStateIndx_; }

  StateIndx getHaltIndx() const noexcept { return haltStateIndx_; }

  StateConstIter begin() const noexcept { return states_.begin(); }

  StateConstIter end() const noexcept { return states_.end(); }
};

class Tape final {
  using Symbol = States::Symbol;
  using SymbolStorage = std::deque<Symbol>;
  using StateIndx = States::StateIndx;

  Symbol head_;
  StateIndx curStateIndx_;
  SymbolStorage left_;
  SymbolStorage right_;

  static size_t toNumber(const SymbolStorage &symbols) {
    size_t num = 0;
    for (size_t i = 0; i != symbols.size(); ++i) {
      if (symbols[i] == true) {
        num += 1 << (symbols.size() - i - 1);
      }
    }
    return num;
  }

public:
  void read(std::istream &is, const States &states) {
    std::string pat, tapeStr;
    is >> pat;
    utils::checkPattern(pat, "initial:");
    std::getline(is, pat);
    is >> tapeStr;
    utils::checkPattern(tapeStr, "[");
    utils::checkPattern(tapeStr, "]");
    auto leftBound = tapeStr.begin() + tapeStr.find('[');
    auto rightBound = tapeStr.begin() + tapeStr.find(']');
    assert(leftBound < rightBound);
    for (auto it = tapeStr.begin(); it != std::prev(leftBound); ++it)
      left_.emplace_back(strToSym(*it));
    head_ = strToSym(*std::prev(leftBound));
    for (auto it = std::next(rightBound); it != tapeStr.end(); ++it)
      right_.emplace_back(strToSym(*it));
    auto curStateName = std::string(std::next(leftBound), rightBound);
    curStateIndx_ = states.getState(curStateName).indx_;
  }

  void dump(std::ostream &os, const States &states) const {
    for (auto &&sym : left_)
      os << symToStr(sym);
    os << symToStr(head_);
    os << '[' << states.getState(curStateIndx_).name_ << ']';
    for (auto &&sym : right_)
      os << symToStr(sym);
    os << "\n";
  }

  void moveLeft(Symbol newSym) {
    right_.emplace_front(newSym);
    if (!left_.empty()) {
      head_ = left_.back();
      left_.pop_back();
    } else {
      head_ = false;
    }
    if (right_.back() == false)
      right_.pop_back();
  }

  void moveRight(Symbol newSym) {
    left_.emplace_back(newSym);
    if (!right_.empty()) {
      head_ = right_.front();
      right_.pop_front();
    } else {
      head_ = false;
    }
    if (left_.front() == false)
      left_.pop_front();
  }

  size_t leftNumber() const { return toNumber(left_); }

  size_t rightNumber() const { return toNumber(right_); }

  void setCurStateIndx(StateIndx curStateIndx) { curStateIndx_ = curStateIndx; }

  StateIndx getCurStateIndx() const { return curStateIndx_; }

  Symbol getHead() const { return head_; }

private:
  static Symbol strToSym(char sym) {
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

  static char symToStr(bool sym) { return sym ? '1' : '0'; }
};

class TuringMachine final : public machines::Machine<TuringMachine> {
  using Symbol = States::Symbol;
  using StateVal = States::StateIndx;

  States states_;
  Tape tape_;

  void read(std::istream &is) {
    states_.read(is);
    tape_.read(is, states_);
  }

  void dumpTable(std::ostream &os) const { states_.dump(os); }

  void dumpState(std::ostream &os) const { tape_.dump(os, states_); }

  void step() {
    const auto &state = states_.getState(tape_.getCurStateIndx());
    const auto &jump = state.jumps_[tape_.getHead()];
    switch (jump.move_) {
    case States::Move::L:
      tape_.moveLeft(jump.newSym_);
      break;
    case States::Move::R:
      tape_.moveRight(jump.newSym_);
      break;
    }
    tape_.setCurStateIndx(jump.newStateIndx_);
  }

  bool hlt() const noexcept { return states_.isHlt(tape_.getCurStateIndx()); }

  friend class machines::TMConverter;

public:
  void execute(std::istream &is, std::ostream &os, DumpLvl lvl) {
    read(is);
    dumpTable(os);
    for (;;) {
      if (hlt())
        break;
      if (lvl > 0)
        dumpState(os);
      step();
    }
    dumpState(os);
    os.flush();
  }
};

} // namespace tm

} // namespace machines
