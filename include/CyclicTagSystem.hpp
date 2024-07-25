#pragma once

#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_set>

#include "Utils.hpp"
#include "AbstractMachine.hpp"

namespace machines {

class TSConverter;

namespace cts {

using utils::checkPattern;
using utils::readLineToSS;

class Tags final {
public:
  using TagVal = std::deque<bool>;

  static TagVal getTag(std::string_view tagStr) {
    TagVal tag;
    if (tagStr == "-") {
      return tag;
    }
    for (auto&& sym : tagStr) {
      tag.emplace_back(toBool(sym));
    }
    return tag;
  }

  static std::string dumpTag(const TagVal& tag) {
    if (tag.empty()) {
      return "-";
    }
    std::string tagStr;
    for (auto&& sym : tag) {
      tagStr.push_back(toChar(sym));
    }
    return tagStr;
  }

  static char toChar(bool sym) noexcept { return sym ? 'Y' : 'N'; }

private:
  struct TagHash {
    auto operator()(const TagVal& rhs) const {
      auto str = dumpTag(rhs);
      return std::hash<std::string>{}(str);
    }
  };

  struct TagEqual {
    auto operator()(const TagVal& lhs, const TagVal& rhs) const {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
  };

  std::vector<TagVal> tags_;
  std::unordered_set<TagVal, TagHash, TagEqual> halts_;

  static bool toBool(char sym) {
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

  void readTable(std::istream& is) {
    std::string pat, tagStr;
    is >> pat;
    checkPattern(pat, "table:");
    std::getline(is, pat);
    auto iss = readLineToSS(is);
    while(iss >> tagStr) {
      tags_.emplace_back(getTag(tagStr));
    }
  }

  bool checkHaltsSize() const {
    auto haltSize = halts_.begin()->size();
    return std::all_of(halts_.begin(), halts_.end(), [&](auto&& it) {
      return it.size() == haltSize;
    });
  }

  void readHalt(std::istream& is) {
    std::string pat, tagStr;
    is >> pat;
    checkPattern(pat, "halt:");
    std::getline(is, pat);
    auto iss = readLineToSS(is);
    while (iss >> tagStr) {
      auto tag = getTag(tagStr);
      halts_.emplace(tag);
    }
    if (!checkHaltsSize()) {
      auto msg = "Halts tags have different size";
      throw std::runtime_error(msg);
    }
  }

  void dumpTable(std::ostream& os) const {
    os << "table:\n";
    for (auto&& tag : tags_) {
      os << dumpTag(tag) << " ";
    }
    os << "\n\n";
  }

  void dumpHalt(std::ostream& os) const {
    os << "halt:\n";
    for (auto&& tag : halts_) {;
      os << dumpTag(tag) << " ";
    }
    os << "\n\n";
  }

public:
  void read(std::istream& is) {
    readTable(is);
    readHalt(is);
  }

  void dump(std::ostream&os) const {
    dumpTable(os);
    dumpHalt(os);
  }

  size_t size() const { return tags_.size(); }

  const TagVal& getTag(size_t indx) const { return tags_.at(indx); }

  bool hlt(const TagVal& tag) const {
    auto haltSize = halts_.begin()->size();
    if (tag.size() < haltSize) {
      return false;
    }
    auto cutTag = TagVal{tag.begin(), tag.begin() + haltSize};
    return halts_.count(cutTag);
  }

  auto haltBegin() const noexcept { return halts_.begin(); }

  auto haltEnd() const noexcept { return halts_.end(); }

  auto begin() const noexcept { return tags_.begin(); }

  auto end() const noexcept { return tags_.end(); }
};

struct Queue final {
  std::deque<bool> queue_;
  size_t indx_;

  void read(std::istream& is, const Tags& tags) {
    std::string pat, init;
    is >> pat;
    checkPattern(pat, "initial:");
    std::getline(is, pat);
    is >> init;
    queue_ = tags.getTag(init);
    indx_ = 0U;
  }

  void dump(std::ostream& os, const Tags& tags) const {
    os << tags.dumpTag(queue_) << "\n";
  }
};

class CyclicTagSystem : public Machine<CyclicTagSystem> {
  using TagVal = Tags::TagVal;

  Tags tags_;
  Queue queue_;

  void read(std::istream& is) {
    tags_.read(is);
    queue_.read(is, tags_);
  }

  void dumpTable(std::ostream& os) const {
    tags_.dump(os);
  }

  void dumpState(std::ostream& os) const {
    queue_.dump(os, tags_);
    os.flush();
  }

  void step() {
    auto& queue = queue_.queue_;
    if (queue.empty()) {
      auto msg = "Queue is empty";
      throw std::runtime_error(msg);
    }
    auto head = queue.front();
    queue.pop_front();
    if (head) {
      auto& tag = tags_.getTag(queue_.indx_);
      queue.insert(queue.end(), tag.begin(), tag.end());
    }
    queue_.indx_ = (queue_.indx_ + 1) % tags_.size();
  }

  bool atBegin() const {
    return queue_.indx_ == 0U;
  }

  bool hlt() const {
    return tags_.hlt(queue_.queue_);
  }

  friend class machines::TSConverter;

public:
  void execute(std::istream &is, std::ostream& os, DumpLvl lvl) {
      read(is);
      dumpTable(os);
      for (;;) {
        auto begin = atBegin();
        if (hlt() && begin) {
          break;
        }
        if (lvl > 0) {
          if (begin || lvl > 1) {
            dumpState(os);
          }
        }
        step();
      }
      dumpState(os);
      os.flush();
  }
};


} // namespace cts

} // namespace machines
