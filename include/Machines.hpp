#pragma once

#include "States.hpp"

namespace machines {

struct Tape {
  int stateVal_;
  Symbol head_;
  std::deque<Symbol> left_;
  std::deque<Symbol> right_;

  static Tape read(std::istream &is, const States &states) {
    Tape tape_;
    std::string tape;
    if (!(is >> tape)) {
      throw std::runtime_error("Can't read tape");
    }
    auto leftBound = tape.begin() + tape.find('[');
    auto rightBound = tape.begin() + tape.find(']');
    for (auto it = tape.begin(); it != leftBound; ++it) {
      tape_.left_.emplace_back(toSymbol(*it));
    }
    tape_.left_.pop_back();
    tape_.head_ = toSymbol(*std::prev(leftBound));
    for (auto it = std::next(rightBound); it != tape.end(); ++it) {
      tape_.right_.emplace_back(toSymbol(*it));
    }
    tape_.stateVal_ =
        states.getStateVal(std::string(std::next(leftBound), rightBound));
    return tape_;
  }

  void dump(std::ostream &os, const States &states) const {
    for (auto &&sym : left_) {
      os << toChar(sym);
    }
    os << toChar(head_);
    os << '[' << states.getStateName(stateVal_) << ']';
    for (auto &&sym : right_) {
      os << toChar(sym);
    }
    os << std::endl;
  }
};

class TuringMachine {
  States states_;
  Tape tape_;

public:
  TuringMachine(const States &states, const Tape &tape)
      : states_(states), tape_(tape) {}

  void makeStep() {
    auto headSym = tape_.head_;
    auto stateVal = tape_.stateVal_;
    auto move = states_.getJumpMove(stateVal, headSym);
    auto write = states_.getJumpWriteSymbol(stateVal, headSym);
    auto newStateVal = states_.getJumpNewStateVal(stateVal, headSym);
    auto tape = tape_;
    switch (move) {
    case Move::L:
      tape.right_.emplace_front(write);
      if (!tape.left_.empty()) {
        tape.head_ = tape.left_.back();
        tape.left_.pop_back();
      } else {
        tape.head_ = false;
      }
      break;
    case Move::R:
      tape.left_.emplace_back(write);
      if (!tape.right_.empty()) {
        tape.head_ = tape.right_.front();
        tape.right_.pop_front();
      } else {
        tape.head_ = false;
      }
    }
    tape.stateVal_ = newStateVal;
    tape_ = std::move(tape);
  }

  void run(std::ostream &os) {
    tape_.dump(os, states_);
    for (;;) {
      makeStep();
      tape_.dump(os, states_);
      if (states_.isHltType(tape_.stateVal_)) {
        break;
      }
    }
  }
};

enum class TagType {
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

class Tag {
  int k_;
  TagType type_;

public:
  Tag() = default;

  Tag(int k, TagType type) : k_(k), type_(type) {}

  auto operator<=>(const Tag &rhs) const = default;

  TagType getType() const { return type_; }

  int getK() const { return k_; }

  std::string toString(const std::string &k) const {
    switch (type_) {
    case TagType::Lk:
      return "L" + k;
    case TagType::Lk0:
      return "L" + k + "0";
    case TagType::Lk1:
      return "L" + k + "1";
    case TagType::Rk:
      return "R" + k;
    case TagType::Rk0:
      return "R" + k + "0";
    case TagType::Rk1:
      return "R" + k + "1";
    case TagType::Rkk:
      return "R" + k + k;
    case TagType::Hk:
      return "H" + k;
    case TagType::Hk0:
      return "H" + k + "0";
    case TagType::Hk1:
      return "H" + k + "1";
    }
  }
};

struct TagSystem {
  States states_;

  int stateVal_;
  Symbol head_;
  std::deque<Tag> queue_;
  bool a_;
  bool b_;
  bool c_;
  bool d_;
  bool e_;

  void addNext(Tag tag, int newK) {
    auto k = tag.getK();
    switch (tag.getType()) {
    case TagType::Lk:
      queue_.emplace_back(k, TagType::Lk1);
      queue_.emplace_back(k, TagType::Lk0);
      break;
    case TagType::Lk0:
    case TagType::Lk1:
      queue_.emplace_back(newK, TagType::Lk);
      if (d_) {
        queue_.emplace_back(newK, TagType::Lk);
        queue_.emplace_back(newK, TagType::Lk);
        queue_.emplace_back(newK, TagType::Lk);
      }
      break;
    case TagType::Rk:
      queue_.emplace_back(k, TagType::Rk1);
      queue_.emplace_back(k, TagType::Rk0);
      break;
    case TagType::Rk0:
    case TagType::Rk1:
      queue_.emplace_back(newK, TagType::Rk);
      if (e_) {
        queue_.emplace_back(newK, TagType::Rk);
        queue_.emplace_back(newK, TagType::Rk);
        queue_.emplace_back(newK, TagType::Rk);
      }
      break;
    case TagType::Rkk:
      queue_.emplace_back(k, TagType::Rk);
      queue_.emplace_back(k, TagType::Rk);
      break;
    case TagType::Hk:
      queue_.emplace_back(k, TagType::Hk1);
      queue_.emplace_back(k, TagType::Hk0);
      break;
    case TagType::Hk0:
    case TagType::Hk1:
      if (a_) {
        queue_.emplace_back(newK, TagType::Rkk);
        queue_.emplace_back(newK, TagType::Rkk);
      }
      if (b_) {
        queue_.emplace_back(newK, TagType::Hk);
      }
      queue_.emplace_back(newK, TagType::Hk);
      if (c_) {
        queue_.emplace_back(newK, TagType::Lk);
        queue_.emplace_back(newK, TagType::Lk);
      }
      break;
    }
  }

  size_t toNumber(const std::deque<bool> &queue) {
    size_t num = 0;
    for (auto i = 0; i < queue.size(); ++i) {
      if (queue[i]) {
        num += 1 << (queue.size() - i - 1);
      }
    }
    return num;
  }

  void clearFlags() {
    a_ = false;
    b_ = false;
    c_ = false;
    d_ = false;
    e_ = false;
  }

  void fillHead() {
    switch (queue_.front().getType()) {
    case TagType::Hk0:
      head_ = false;
      break;
    case TagType::Hk1:
      head_ = true;
      break;
    default:
      throw std::runtime_error("Queue starts with incorrect tag");
    }
  }

  void fillFlags() {
    auto move = states_.getJumpMove(stateVal_, head_);
    auto write = states_.getJumpWriteSymbol(stateVal_, head_);
    clearFlags();
    if (move == Move::L) {
      if (write == true) {
        a_ = true;
      }
      e_ = true;
    }
    if (head_ == false) {
      b_ = true;
    }
    if (move == Move::R) {
      if (write == true) {
        c_ = true;
      }
      d_ = true;
    }
  }

  std::deque<Tag> fillQueue(const Tape &tape) {
    stateVal_ = tape.stateVal_;
    auto k = stateVal_;
    std::deque<Tag> queue;
    auto left = toNumber(tape.left_);
    auto right = toNumber(tape.right_);
    if (tape.head_) {
      queue.emplace_back(k, TagType::Hk1);
      queue.emplace_back(k, TagType::Hk0);
      for (auto i = 0; i < left; ++i) {
        queue.emplace_back(k, TagType::Lk1);
        queue.emplace_back(k, TagType::Lk0);
      }
      for (auto i = 0; i < right; ++i) {
        queue.emplace_back(k, TagType::Rk1);
        queue.emplace_back(k, TagType::Rk0);
      }
    } else {
      queue.emplace_back(k, TagType::Hk0);
      queue.emplace_back(k, TagType::Hk1);
      for (auto i = 0; i < left; ++i) {
        queue.emplace_back(k, TagType::Lk0);
        queue.emplace_back(k, TagType::Lk1);
      }
      for (auto i = 0; i < right; ++i) {
        queue.emplace_back(k, TagType::Rk0);
        queue.emplace_back(k, TagType::Rk1);
      }
      queue.pop_back();
    }
    return queue;
  }

public:
  TagSystem(const States &states, const Tape &tape) : states_(states) {
    queue_ = std::move(fillQueue(tape));
  }

  void makeStep() {
    fillHead();
    fillFlags();
    auto newStateVal = states_.getJumpNewStateVal(stateVal_, head_);
    for (;;) {
      auto poped = queue_.front();
      queue_.pop_front();
      queue_.pop_front();
      addNext(poped, newStateVal);
      auto headType = queue_.front().getType();
      if (headType == TagType::Hk0 || headType == TagType::Hk1) {
        break;
      }
    }
    stateVal_ = newStateVal;
  }

  void run(std::ostream &os) {
    dump(os);
    for (;;) {
      makeStep();
      dump(os);
      if (states_.isHltType(stateVal_)) {
        break;
      }
    }
  }

  void dump(std::ostream &os) {
    using iter = std::deque<Tag>::iterator;
    using tag_pair = std::pair<iter, iter>;

    auto stateName = states_.getStateName(stateVal_);
    size_t repeatCount = 0;
    tag_pair prev = {queue_.begin(), queue_.begin() + 1};
    tag_pair cur = prev;

    auto dumpPair = [&](const tag_pair &pair, size_t repeat) -> void {
      if (repeat == 0) {
        return;
      }
      os << "(" << prev.first->toString(stateName);
      os << prev.second->toString(stateName) << ")";
      if (repeat > 1) {
        os << "[" << std::to_string(repeat) << "]";
      }
    };
    // interesting error "zsh: illegal hardware instruction  ./build/converter"
    // with this code instead:
    //    for (; cur.first != queue_.end() || cur.second != queue_.end();
    //           cur.first += 2, cur.second += 2) {
    for (;;) {
      if (*cur.first == *prev.first && *cur.second == *prev.second) {
        repeatCount++;
      } else {
        dumpPair(prev, repeatCount);
        repeatCount = 1;
        prev = cur;
      }
      if (cur.first == queue_.end() || cur.second == queue_.end()) {
        if (cur.first != queue_.end()) {
          dumpPair(prev, repeatCount - 1);
          os << cur.first->toString(stateName);
        }
        break;
      }
      cur.first += 2;
      cur.second += 2;
    }
    os << std::endl;
  }
};

} // namespace machines
