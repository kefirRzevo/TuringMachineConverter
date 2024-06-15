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

enum class Move : bool {
  L,
  R,
};

Move stringToMove(const std::string& move) {
    if (move == "L") {
      return Move::L;
    } else if (move == "R") {
      return Move::R;
    } else {
      throw std::runtime_error("Can't read move " + move);
    }
}

std::string moveToString(Move move) {
  switch (move) {
    case Move::L: return "L";
    case Move::R: return "R";
  }
}

using Symbol = bool;
const size_t SymbolCount = 2;

Symbol symbolFromChar(char sym) {
  switch (sym) {
    case '0': return false;
    case '1': return true;
    default: 
    std::cerr << "[" << sym << "]" << std::endl;
    throw std::runtime_error("Unknown symbol " + std::string(1, sym));
  }
}

char charFromSymbol(Symbol sym) {
  return sym ? '1' : '0';
}

struct Jump {
  enum class Type : bool {
    Known,
    Unknown,
  };

  Type type_;
  int newState_;
  Symbol write_;
  Move move_;

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
    os << "move " << moveToString(move_) << std::endl;
  }

  bool validate() const { return type_ == Type::Known; }
};

struct State {
  enum class Type {
    Hlt,
    Known,
    Unknown,
  };

  Type type_;
  int val_;
  std::array<Jump, SymbolCount> jumps_;

  State() : type_(Type::Unknown) {}

  State(int val) : type_(Type::Unknown), val_(val) {}

  void setJumpNewState(size_t pos, int newState) {
    jumps_[pos].setNewState(newState);
    auto valid = std::all_of(jumps_.begin(), jumps_.end(), [](auto&& jump) {
      return jump.validate(); }
    );
    if (valid) { type_ = Type::Known; }
  }

  int getJumpNewState(size_t pos) const {
    return jumps_[pos].getNewState();
  }

  Symbol getJumpWriteSymbol(size_t pos) const {
    return jumps_[pos].getWriteSymbol();
  }

  Move getJumpMove(size_t pos) const { return jumps_[pos].getMove(); }

  int getVal() const { return val_; }

  void setJump(size_t pos, Jump&& jump) { jumps_[pos] = std::move(jump); }

  bool validate() const { return type_ != Type::Unknown; }

  void dump(std::ostream& os) const {
    os << "Val " << val_ << std::endl;
    for (auto i = 0; i != SymbolCount; ++i) {
      os << "\tsym " << i << " ";
      jumps_[i].dump(os);
    }
  }
};

struct TuringMachine {
  std::unordered_map<std::string, State> states_;

  struct Tape {
    State state_;
    Symbol head_;
    std::deque<Symbol> left_;
    std::deque<Symbol> right_;
  };

  Tape init_;
  Tape cur_;

  TuringMachine(std::istream& is) {
    auto& hlt = getState("hlt");
    hlt.type_ = State::Type::Hlt;
    readStates(is);
    readTape(is);
    cur_ = init_;
    dumpStates(std::cout);
    dumpTape(std::cout);
    if (!validate()) {
      throw std::runtime_error("Invalid machine");
    }
  }

  const std::string& getStateName(int val) const {
    auto found = std::find_if(states_.begin(), states_.end(), [&](auto&& state) {
      return state.second.getVal() == val; }
    );
    if (found != states_.end()) { return found->first; }
    throw std::runtime_error("Can't find state " + std::to_string(val));
  }

  State& getState(const std::string& stateName) {
      auto stateIt = states_.find(stateName);
      if (stateIt == states_.end()) {
        auto [found, flag] = states_.emplace(stateName, states_.size());
        stateIt = found;
        if (!flag) {
          throw std::runtime_error("Can't emplace state " + std::string(stateName));
        }
      }
      return stateIt->second;
  }

  State& getExistState(const std::string& stateName) {
      auto stateIt = states_.find(stateName);
      if (stateIt != states_.end()) {
        return stateIt->second;
      }
      throw std::runtime_error("Can't find state " + stateName);
  }

  void readStates(std::istream& is) {
    using fill_val = std::tuple<State*, size_t, std::string>;

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
        auto& state = getState(stateName);
        state.setJump(sym, Jump{symWrite, stringToMove(move)});
        toFill.emplace_back(std::addressof(state), sym, stateNext);
      } else {
        is.clear();
        is.seekg(prevPos);
        break;
      }
    }
    for (auto&& [state, jumpPos, next] : toFill) {
      auto& nextState = getExistState(next);
      state->setJumpNewState(jumpPos, nextState.getVal());
    }
  }

  void readTape(std::istream& is) {
    std::string tape;
    if(!(is >> tape)) {
      throw std::runtime_error("Can't read tape");
    }
    auto leftBound = tape.begin() + tape.find('[');
    auto rightBound = tape.begin() + tape.find(']');
    for (auto it = tape.begin(); it != leftBound; ++it) {
      init_.left_.emplace_front(symbolFromChar(*it));
    }
    init_.head_ = symbolFromChar(*std::prev(leftBound));
    init_.state_ = getExistState(std::string(std::next(leftBound), rightBound));
    for (auto it = std::next(rightBound); it != tape.end(); ++it) {
      init_.right_.emplace_back(symbolFromChar(*it));
    }
  }

  bool validate() const {
    return std::all_of(states_.begin(), states_.end(), [](auto&& state) {
      return state.second.validate();
    });
  }

  void dumpStates(std::ostream& os) {
    for (auto&& state : states_) {
      os << "State " << state.first << " ";
      state.second.dump(std::cout);
    }
  }

  void dumpTape(std::ostream& os) {
    for (auto it = cur_.left_.rbegin(); it != cur_.left_.rend(); ++it) {
      os << charFromSymbol(*it);
    }
    os << '[' << getStateName(cur_.state_.val_) << ']';
    for (auto it = cur_.right_.begin(); it != cur_.right_.end(); ++it) {
      os << charFromSymbol(*it);
    }
    os << std::endl;
  }

  State& getNextState() {
    Tape next = cur_;
    auto curHeadSym = cur_.head_;
    auto nextState = cur_.state_.getJumpNewState(curHeadSym);
    switch (curHeadSym) {
      case true:
    }
  } 
};

enum class Tags {
  Lk,
  Lk0,
  Lk1,
  Rk,
  Rk0,
  Rk1,
  Rkk,
  Hk,
  Hk0,
  Hk1,
};



struct TagSystem {

};

class Emulator {
  TuringMachine machine_;
  TagSystem system_;


};
