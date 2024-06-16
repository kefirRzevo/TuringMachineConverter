#pragma once

#include <algorithm>
#include <deque>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "AbstractMachine.hpp"
#include "Utils.hpp"

namespace machines {

class TSConverter;

namespace cts {

class Tags final {
public:
  using Symbol = bool;
  using Tag = std::deque<Symbol>;

  static Tag strToTag(std::string_view str) {
    Tag tag;
    if (str == "-")
      return tag;
    for (auto &&sym : str)
      tag.emplace_back(strToSym(sym));
    return tag;
  }

  static std::string tagToStr(const Tag &tag) {
    if (tag.empty())
      return "-";
    std::string tagStr;
    for (auto &&sym : tag)
      tagStr.push_back(symToStr(sym));
    return tagStr;
  }

  static char symToStr(Symbol sym) noexcept { return sym ? 'Y' : 'N'; }

  static Symbol strToSym(char sym) {
    switch (sym) {
    case 'Y':
      return true;
    case 'N':
      return false;
    default:
      auto msg = "Unknown symbol '" + std::string(1, sym) + "'";
      throw std::runtime_error(msg);
    }
  }

private:
  struct TagHash {
    size_t operator()(const Tag &rhs) const {
      return std::hash<std::string>{}(tagToStr(rhs));
    }
  };

  struct TagEqual {
    bool operator()(const Tag &lhs, const Tag &rhs) const {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
  };

  using TagStorage = std::vector<Tag>;
  using HaltTagStorage = std::unordered_set<Tag, TagHash, TagEqual>;

  TagStorage tags_;
  HaltTagStorage halts_;

  void readTable(std::istream &is) {
    std::string pat, tagStr;
    is >> pat;
    utils::checkPattern(pat, "table:");
    std::getline(is, pat);
    auto iss = utils::readLineToSS(is);
    while (iss >> tagStr)
      tags_.emplace_back(strToTag(tagStr));
  }

  bool checkHaltsSize() const {
    auto haltSize = halts_.begin()->size();
    return std::all_of(halts_.begin(), halts_.end(),
                       [&](auto &&it) { return it.size() == haltSize; });
  }

  void readHalt(std::istream &is) {
    std::string pat, tagStr;
    is >> pat;
    utils::checkPattern(pat, "halt:");
    std::getline(is, pat);
    auto iss = utils::readLineToSS(is);
    while (iss >> tagStr)
      halts_.emplace(strToTag(tagStr));
    if (!checkHaltsSize()) {
      auto msg = "Halts tags have different size";
      throw std::runtime_error(msg);
    }
  }

  void dumpTable(std::ostream &os) const {
    os << "table:\n";
    for (auto &&tag : tags_)
      os << tagToStr(tag) << " ";
    os << "\n\n";
  }

  void dumpHalt(std::ostream &os) const {
    os << "halt:\n";
    for (auto &&tag : halts_)
      os << tagToStr(tag) << " ";
    os << "\n\n";
  }

public:
  using TagIter = TagStorage::const_iterator;
  using HaltIter = HaltTagStorage::const_iterator;

  void read(std::istream &is) {
    readTable(is);
    readHalt(is);
  }

  void dump(std::ostream &os) const {
    dumpTable(os);
    dumpHalt(os);
  }

  size_t size() const noexcept { return tags_.size(); }

  size_t haltSize() const noexcept { return halts_.begin()->size(); }

  const Tag &getTag(size_t indx) const { return tags_.at(indx); }

  bool isHlt(const Tag &tag) const {
    if (tag.size() < haltSize()) {
      return false;
    }
    auto cutTag = Tag{tag.begin(), tag.begin() + haltSize()};
    return halts_.count(cutTag);
  }

  TagIter begin() const noexcept { return tags_.begin(); }

  TagIter end() const noexcept { return tags_.end(); }

  HaltIter haltBegin() const noexcept { return halts_.begin(); }

  HaltIter haltEnd() const noexcept { return halts_.end(); }
};

class CyclicTagSystem : public Machine<CyclicTagSystem> {
  using Queue = Tags::Tag;

  Tags tags_;
  Queue queue_;
  size_t indx_;

  void read(std::istream &is) {
    tags_.read(is);
    std::string pat, init;
    is >> pat;
    utils::checkPattern(pat, "initial:");
    std::getline(is, pat);
    is >> init;
    queue_ = Tags::strToTag(init);
    indx_ = 0U;
  }

  void dumpTable(std::ostream &os) const { tags_.dump(os); }

  void dumpState(std::ostream &os) const {
    os << Tags::tagToStr(queue_) << std::endl;
  }

  void step() {
    if (queue_.empty()) {
      auto msg = "Queue is empty";
      throw std::runtime_error(msg);
    }
    auto head = queue_.front();
    queue_.pop_front();
    if (head) {
      const auto &tag = tags_.getTag(indx_);
      std::copy(tag.begin(), tag.end(), std::back_inserter(queue_));
    }
    indx_ = (indx_ + 1) % tags_.size();
  }

  bool atBegin() const { return indx_ == 0U; }

  bool hlt() const { return tags_.isHlt(queue_); }

  friend class machines::TSConverter;

public:
  void execute(std::istream &is, std::ostream &os, DumpLvl lvl) {
    read(is);
    dumpTable(os);
    size_t stepsCount = 0U;
    for (;;) {
      auto begin = atBegin();
      if (hlt() && begin)
        break;
      if (lvl > 0)
        if (begin || lvl > 1)
          dumpState(os);
      step();
      stepsCount++;
      if (stepsCount > 10000) {
        auto msg = "Too many steps";
        throw std::runtime_error(msg);
      }
    }
    dumpState(os);
    os.flush();
  }
};

} // namespace cts

} // namespace machines
