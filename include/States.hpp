#pragma once

#include <tuple>
#include <array>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

using Symbol = bool;
const size_t SymbolCount = 2;

enum class Move : bool {
  L,
  R,
};

inline
Move toMove(const std::string& move) {
    if (move == "L") {
      return Move::L;
    } else if (move == "R") {
      return Move::R;
    } else {
      throw std::runtime_error("Can't read move " + move);
    }
}

inline
std::string toString(Move move) {
  switch (move) {
    case Move::L: return "L";
    case Move::R: return "R";
  }
}

inline
Symbol toSymbol(char sym) {
  switch (sym) {
    case '0': return false;
    case '1': return true;
    default: 
    std::cerr << "[" << sym << "]" << std::endl;
    throw std::runtime_error("Unknown symbol " + std::string(1, sym));
  }
}

inline
char toChar(Symbol sym) {
  return sym ? '1' : '0';
}

class Jump final {
  enum class Type : bool {
    Known,
    Unknown,
  };

  Type type_;
  int newState_;
  Symbol write_;
  Move move_;

public:
  Jump() : type_(Type::Unknown) {}

  Jump(Symbol write, Move move)
  : type_(Type::Unknown), write_(write), move_(move) {}

  void setNewState(int newState) {
    newState_ = newState;
    type_ = Type::Known;
  }

  int getNewState() const { return newState_; }

  Symbol getWriteSymbol() const { return write_; }

  Move getMove() const { return move_; }

  void dump(std::ostream& os) const {
    os << "write " << write_ << "; new state " << newState_ << "; ";
    os << "move " << toString(move_) << std::endl;
  }

  bool validate() const { return type_ == Type::Known; }
};

class State final {
  enum class Type {
    Hlt,
    Known,
    Unknown,
  };

  Type type_;
  int val_;
  std::array<Jump, SymbolCount> jumps_;

public:
  State() : type_(Type::Unknown) {}

  State(int val) : type_(Type::Unknown), val_(val) {}

  void setJumpNewState(size_t pos, int newState) {
    jumps_[pos].setNewState(newState);
    auto valid = std::all_of(jumps_.begin(), jumps_.end(), [](auto&& jump) {
      return jump.validate(); }
    );
    if (valid) { type_ = Type::Known; }
  }

  void setJump(size_t pos, Jump&& jump) { jumps_[pos] = std::move(jump); }

  void setHltType() { type_ = Type::Hlt; }

  int getJumpNewStateVal(size_t pos) const { return jumps_[pos].getNewState(); }

  Symbol getJumpWriteSymbol(size_t pos) const {
    return jumps_[pos].getWriteSymbol();
  }

  Move getJumpMove(size_t pos) const { return jumps_[pos].getMove(); }

  int getVal() const { return val_; }

  bool isHltType() const { return type_ == Type::Hlt; }

  bool validate() const { return type_ != Type::Unknown; }

  void dump(std::ostream& os) const {
    os << "Val " << val_ << std::endl;
    for (auto i = 0; i != SymbolCount; ++i) {
      os << "\tsym " << i << " ";
      jumps_[i].dump(os);
    }
  }
};

class States final {
  using state_value = std::pair<std::string, State>;

  std::vector<state_value> states_;

  State& getState(int val) { return states_.at(val).second; }

  const State& getState(int val) const { return states_.at(val).second; }

  States() = default;

  bool validate() const {
    return std::all_of(states_.begin(), states_.end(), [](auto&& state) {
      return state.second.validate();
    });
  }

  int addState(const std::string& stateName) {
      auto found = std::find_if(states_.begin(), states_.end(), [&](auto&& state) {
        return state.first == stateName;
      });
      if (found != states_.end()) {
        return found->second.getVal();
      }
      states_.emplace_back(stateName, states_.size());
      return states_.back().second.getVal();
  }

public:
  static States read(std::istream& is) {
    States states;
    auto hltVal = states.addState("hlt");
    states.getState(hltVal).setHltType();

    using fill_val = std::tuple<int, size_t, std::string>;
    std::string stateName;
    Symbol sym;
    Symbol symWrite;
    std::string stateNext;
    std::string move;
    std::vector<fill_val> toFill;
    auto prevPos = is.tellg();
    for (;;) {
      if (is >> stateName >> sym >> symWrite >> stateNext >> move) {
        prevPos = is.tellg();
        auto stateVal = states.addState(stateName);
        auto& state = states.getState(stateVal);
        state.setJump(sym, Jump{symWrite, toMove(move)});
        toFill.emplace_back(state.getVal(), sym, stateNext);
      } else {
        is.clear();
        is.seekg(prevPos);
        break;
      }
    }
    for (auto&& [stateVal, jumpSym, nextStateName] : toFill) {
      auto nextStateVal = states.getStateVal(nextStateName);
      states.getState(stateVal).setJumpNewState(jumpSym, nextStateVal);
    }
    if (!states.validate()) {
      throw std::runtime_error("Invalid states");
    }
    return states;
  }

  const std::string& getStateName(int val) const { return states_.at(val).first; }

  int getStateVal(const std::string& stateName) const {
    auto found = std::find_if(states_.begin(), states_.end(), [&](auto&& state) {
      return state.first == stateName;
    });
    if (found != states_.end()) {
      return found->second.getVal();
    }
    throw std::runtime_error("Can't find state " + stateName);
  }

  bool isHltType(int stateVal) const {
    return getState(stateVal).isHltType();
  }

  Move getJumpMove(int stateVal, size_t pos) const {
    return getState(stateVal).getJumpMove(pos);
  }

  int getJumpNewStateVal(int stateVal, size_t pos) const {
    return getState(stateVal).getJumpNewStateVal(pos);
  }

  Symbol getJumpWriteSymbol(int stateVal, size_t pos) const {
    return getState(stateVal).getJumpWriteSymbol(pos);
  }

  void dumpStates(std::ostream& os) const {
    for (auto&& state : states_) {
      os << "State " << state.first << " ";
      if (state.second.isHltType()) {
        os << "Val " << state.second.getVal() << std::endl;
        continue;
      }
      state.second.dump(os);
    }
  }

  void dump(std::ostream& os) const {
    for (auto&& state : states_) {
      os << "State " << state.first << " ";
      if (state.second.isHltType()) {
        os << "Val " << state.second.getVal() << std::endl;
        continue;
      }
      state.second.dump(os);
    }
  }
};
