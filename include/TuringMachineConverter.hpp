#pragma once

#include "TuringMachine.hpp"

namespace machines {

struct Tag final {
  using States = tm::States;
  using StateIndx = States::StateIndx;
  using TagStorage = std::vector<Tag>;

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

  StateIndx stateIndx_;
  TagType type_;

  static bool hasZero(TagType type) {
    switch (type) {
    case TagType::Hk0:
    case TagType::Lk0:
    case TagType::Rk0:
      return true;
    default:
      return false;
    }
  }

  static bool hasOne(TagType type) {
    switch (type) {
    case TagType::Hk1:
    case TagType::Lk1:
    case TagType::Rk1:
      return true;
    default:
      return false;
    }
  }

  std::string toString(const States &states) const {
    const auto &state = states.getState(stateIndx_);
    const auto &name = state.name_;
    switch (type_) {
    case TagType::Hk0:
      return "H" + name + "0";
    case TagType::Hk1:
      return "H" + name + "1";
    case TagType::Hk:
      return "H" + name;
    case TagType::Lk0:
      return "L" + name + "0";
    case TagType::Lk1:
      return "L" + name + "1";
    case TagType::Lk:
      return "L" + name;
    case TagType::Rk0:
      return "R" + name + "0";
    case TagType::Rk1:
      return "R" + name + "1";
    case TagType::Rk:
      return "R" + name;
    case TagType::Rkk:
      return "R" + name + name;
    default:
      auto msg = "Unreachable";
      throw std::runtime_error(msg);
    }
  }

  TagStorage getAppends(const States &states) const {
    using Move = tm::States::Move;
    TagStorage appends;
    int i = 0;
    if (hasOne(type_))
      i = 1;
    const auto &state = states.getState(stateIndx_);
    const auto &jump = state.jumps_[i];
    auto newSym = jump.newSym_;
    auto move = jump.move_;
    auto newStateIndx = jump.newStateIndx_;
    auto a = newSym == true && move == Move::L;
    auto b = i == 0;
    auto c = newSym == true && move == Move::R;
    auto d = move == Move::R;
    auto e = move == Move::L;
    switch (type_) {
    case TagType::Hk0:
    case TagType::Hk1: {
      auto tagHk = get(newStateIndx, TagType::Hk);
      auto tagLk = get(newStateIndx, TagType::Lk);
      auto tagRkk = get(newStateIndx, TagType::Rkk);
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
      auto tag1 = get(stateIndx_, TagType::Hk1);
      auto tag0 = get(stateIndx_, TagType::Hk0);
      appends.emplace_back(tag1);
      appends.emplace_back(tag0);
      break;
    }
    case TagType::Lk0:
    case TagType::Lk1: {
      auto tagLk = get(newStateIndx, TagType::Lk);
      if (d) {
        appends.insert(appends.end(), 3, tagLk);
      }
      appends.emplace_back(tagLk);
      break;
    }
    case TagType::Lk: {
      auto tag1 = get(stateIndx_, TagType::Lk1);
      auto tag0 = get(stateIndx_, TagType::Lk0);
      appends.emplace_back(tag1);
      appends.emplace_back(tag0);
      break;
    }
    case TagType::Rk0:
    case TagType::Rk1: {
      auto tagRk = get(newStateIndx, TagType::Rk);
      appends.emplace_back(tagRk);
      if (e) {
        appends.insert(appends.end(), 3, tagRk);
      }
      break;
    }
    case TagType::Rk: {
      auto tag1 = get(stateIndx_, TagType::Rk1);
      auto tag0 = get(stateIndx_, TagType::Rk0);
      appends.emplace_back(tag1);
      appends.emplace_back(tag0);
      break;
    }
    case TagType::Rkk: {
      auto tag = get(stateIndx_, TagType::Rk);
      appends.insert(appends.end(), 2, tag);
      break;
    }
    default:
      auto msg = "Unreachable";
      throw std::runtime_error(msg);
    }
    return appends;
  }

  Tag(StateIndx stateIndx, TagType type) : stateIndx_(stateIndx), type_(type) {}

  static Tag get(StateIndx stateIndx, TagType type) {
    return Tag{stateIndx, type};
  }
};

class TMConverter final : public Converter<TMConverter, tm::TuringMachine> {
  using TuringMachine = tm::TuringMachine;
  using TagType = Tag::TagType;
  using TagStorage = Tag::TagStorage;

  void writeStates(const TuringMachine &tm, std::ostream &os) const {
    auto &&states = tm.states_;
    utils::EnumRange<TagType> range{TagType::Hk0, TagType::Rkk};
    os << "tags:\n";
    for (auto &&state : states) {
      for (auto &&tagType : range) {
        os << Tag::get(state.indx_, tagType).toString(states) << " ";
      }
    }
    os << "\n\n";
  }

  void writeHalt(const TuringMachine &tm, std::ostream &os) const {
    auto &&states = tm.states_;
    auto haltStateIndx = states.getHaltIndx();
    TagStorage halts;
    halts.emplace_back(Tag::get(haltStateIndx, TagType::Hk0));
    halts.emplace_back(Tag::get(haltStateIndx, TagType::Hk1));
    halts.emplace_back(Tag::get(haltStateIndx, TagType::Lk0));
    halts.emplace_back(Tag::get(haltStateIndx, TagType::Lk1));
    halts.emplace_back(Tag::get(haltStateIndx, TagType::Rk0));
    halts.emplace_back(Tag::get(haltStateIndx, TagType::Rk1));
    os << "halt:\n";
    for (auto &&halt : halts)
      os << halt.toString(states) << " ";
    os << "\n\n";
  }

  void writeTable(const TuringMachine &tm, std::ostream &os) const {
    auto &&states = tm.states_;
    utils::EnumRange<TagType> range{TagType::Hk0, TagType::Rkk};
    os << "table:\n";
    for (auto &&state : states) {
      for (auto &&tagType : range) {
        if (states.isHlt(state.indx_))
          if (Tag::hasOne(tagType) || Tag::hasZero(tagType))
            continue;
        auto tag = Tag::get(state.indx_, tagType);
        auto appends = tag.getAppends(states);
        os << tag.toString(states) << " -> ";
        for (auto &&tagToAppend : appends)
          os << tagToAppend.toString(states) << " ";
        os << "\n";
      }
    }
    os << "\n\n";
  }

  void writeInitial(const TuringMachine &tm, std::ostream &os) const {
    auto &&states = tm.states_;
    auto &&tape = tm.tape_;
    auto curStateIndx = tape.getCurStateIndx();
    auto tagHk0 = Tag::get(curStateIndx, TagType::Hk0);
    auto tagHk1 = Tag::get(curStateIndx, TagType::Hk1);
    auto tagLk0 = Tag::get(curStateIndx, TagType::Lk0);
    auto tagLk1 = Tag::get(curStateIndx, TagType::Lk1);
    auto tagRk0 = Tag::get(curStateIndx, TagType::Rk0);
    auto tagRk1 = Tag::get(curStateIndx, TagType::Rk1);
    TagStorage appends;
    os << "initial:\n";
    if (tape.getHead() == true) {
      appends.emplace_back(tagHk1);
      appends.emplace_back(tagHk0);
      for (size_t i = 0; i != tape.leftNumber(); ++i) {
        appends.emplace_back(tagLk1);
        appends.emplace_back(tagLk0);
      }
      for (size_t i = 0; i != tape.rightNumber(); ++i) {
        appends.emplace_back(tagRk1);
        appends.emplace_back(tagRk0);
      }
    } else {
      appends.emplace_back(tagHk0);
      appends.emplace_back(tagHk1);
      for (size_t i = 0; i != tape.leftNumber(); ++i) {
        appends.emplace_back(tagLk0);
        appends.emplace_back(tagLk1);
      }
      for (size_t i = 0; i != tape.rightNumber(); ++i) {
        appends.emplace_back(tagRk0);
        appends.emplace_back(tagRk1);
      }
      appends.pop_back();
    }
    for (auto &&tag : appends)
      os << tag.toString(states) << " ";
    os << "\n";
  }

public:
  void convert(std::istream &is, std::ostream &os) {
    TuringMachine tm;
    tm.read(is);
    writeStates(tm, os);
    writeHalt(tm, os);
    writeTable(tm, os);
    writeInitial(tm, os);
  }
};

} // namespace machines
