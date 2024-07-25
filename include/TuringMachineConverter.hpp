#pragma once

#include "TuringMachine.hpp"

namespace machines {

class TMConverter final : public Converter<TMConverter, tm::TuringMachine> {
  using TuringMachine = tm::TuringMachine;
  using States = tm::States;
  using Tape = tm::Tape;
  using Symbol = TuringMachine::Symbol;
  using StateVal = TuringMachine::StateVal;
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

  static std::string getTag(std::string_view stateStr, TagType type) {
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

  static bool hasOne(TagType tagType) {
    switch(tagType) {
      case TagType::Hk1:
      case TagType::Lk1:
      case TagType::Rk1:
        return true;
      default:
        return false;
    }
  }

  static bool hasZero(TagType tagType) {
    switch(tagType) {
      case TagType::Hk0:
      case TagType::Lk0:
      case TagType::Rk0:
        return true;
      default:
        return false;
    }
  }

  std::vector<std::string> getAppends(const States& states, const State& state,
        std::string_view stateStr, TagType type) const {
    std::vector<std::string> appends;
    auto i = 0;
    if (hasOne(type)) {
      i = 1;
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
    utils::EnumRange<TagType> range{TagType::Hk0, TagType::Rkk};
    os << "tags:\n";
    for (auto&& [stateStr, state] : tm.states_) {
      for (auto&& tagType : range) {
        os << getTag(stateStr, tagType) << " ";
      }
    }
    os << "\n\n";
  }

  void writeHalt(const TuringMachine& tm, std::ostream& os) const {
    auto&& states = tm.states_;
    auto&& haltStr = states.getStateStr(states.getHalt());
    std::vector<std::string> halts;
    halts.emplace_back(getTag(haltStr, TagType::Hk0));
    halts.emplace_back(getTag(haltStr, TagType::Hk1));
    halts.emplace_back(getTag(haltStr, TagType::Lk0));
    halts.emplace_back(getTag(haltStr, TagType::Lk1));
    halts.emplace_back(getTag(haltStr, TagType::Rk0));
    halts.emplace_back(getTag(haltStr, TagType::Rk1));
    os << "halt:\n";
    for (auto&& halt : halts) {
      os << halt << " ";
    }
    os << "\n\n";
  }

  void writeTable(const TuringMachine& tm, std::ostream& os) const {
    utils::EnumRange<TagType> range{TagType::Hk0, TagType::Rkk};
    os << "table:\n";
    for (auto&& [stateStr, state] : tm.states_) {
      for (auto&& tagType : range) {
        if (tm.states_.hlt(state.state_)) {
          if (hasOne(tagType) || hasZero(tagType)) {
            continue;
          }
        }
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
  void convert(std::istream& is, std::ostream& os) {
    TuringMachine tm;
    tm.read(is);
    writeStates(tm, os);
    writeHalt(tm, os);
    writeTable(tm, os);
    writeInitial(tm, os);
  }
};

} //namespace machines
