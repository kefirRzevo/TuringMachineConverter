#pragma once

#include <deque>
#include <vector>
#include <iostream>
#include <algorithm>

#include "Utils.hpp"
#include "AbstractMachine.hpp"

namespace machines {

namespace detail {

using utils::checkPattern;
using utils::readLineToSS;

class Tags {
public:
  using TagVal = unsigned int;

  struct Tag {
    TagVal val_;
    std::vector<TagVal> append_;
  };

private:
  using TagPair = std::pair<std::string, Tag>;

  std::vector<TagPair> tags_;
  std::vector<TagVal> halts_;

  auto findTag(std::string_view tagStr) {
      auto found = std::find_if(tags_.begin(), tags_.end(),
        [&](auto&& tagPair) { return tagPair.first == tagStr; }
      );
      if (found == tags_.end()) {
        auto msg = "Can't find tag '" + std::string(tagStr) + "'";
        throw std::runtime_error(msg);
      }
      return found;
  }

  auto findTag(std::string_view tagStr) const {
      auto found = std::find_if(tags_.begin(), tags_.end(),
        [&](auto&& tagPair) { return tagPair.first == tagStr; }
      );
      if (found == tags_.end()) {
        auto msg = "Can't find tag '" + std::string(tagStr) + "'";
        throw std::runtime_error(msg);
      }
      return found;
  }

  void readTags(std::istream& is) {
    std::string pat, tagStr;
    is >> pat;
    checkPattern(pat, "tags:");
    std::getline(is, pat);
    auto iss = readLineToSS(is);
    while (iss >> tagStr) {
      tags_.emplace_back(tagStr, Tag{});
      tags_.back().second.val_ = tags_.size() - 1;
    }
  }

  void readHalt(std::istream& is) {
    std::string pat, tagStr;
    is >> pat;
    checkPattern(pat, "halt:");
    std::getline(is, pat);
    auto iss = readLineToSS(is);
    while (iss >> tagStr) {
      auto& tag = getTag(tagStr);
      halts_.emplace_back(tag.val_);
    }
    std::sort(halts_.begin(), halts_.end());
  }

  void readTable(std::istream& is) {
    std::string pat, tagStr;
    is >> pat;
    checkPattern(pat, "table:");
    std::getline(is, pat);
    for (auto i = 0; i != tags_.size(); ++i) {
      auto iss = readLineToSS(is);
      iss >> tagStr >> pat;
      checkPattern(pat, "->");
      auto& tag = getTag(tagStr);
      while (iss >> tagStr) {
        tag.append_.emplace_back(getTag(tagStr).val_);
      }
    }
  }

  void dumpTags(std::ostream& os) const {
    os << "tags:\n";
    for (auto && tag : tags_) {
      os << tag.first << " ";
    }
    os << "\n\n";
  }

  void dumpHalt(std::ostream& os) const {
    os << "halt:\n";
    for (auto && halt : halts_) {
      os << getTagStr(halt) << " ";
    }
    os << "\n\n";
  }

  void dumpTable(std::ostream& os) const {
    os << "table:\n";
    for (auto && [tagStr, tag] : tags_) {
      if (hlt(tag.val_)) {
        continue;
      }
      os << tagStr << " -> ";
      for (auto && append : tag.append_) {
        os << getTagStr(append) << " ";
      }
      os << "\n";
    }
    os << "\n\n";
  }

public:
    void read(std::istream& is) {
      readTags(is);
      readHalt(is);
      readTable(is);
    }

    std::string_view getTagStr(TagVal val) const {
      return tags_.at(val).first;
    }

    Tag& getTag(TagVal val) {
      return tags_.at(val).second;
    }

    const Tag& getTag(TagVal val) const {
      return tags_.at(val).second;
    }

    Tag& getTag(std::string_view tagStr) {
      auto found = findTag(tagStr);
      return found->second;
    }

    const Tag& getTag(std::string_view tagStr) const {
      auto found = findTag(tagStr);
      return found->second;
    }

  bool hlt(TagVal val) const {
    return std::binary_search(halts_.begin(), halts_.end(), val);
  }

  void dump(std::ostream& os) const {
    dumpTags(os);
    dumpHalt(os);
    dumpTable(os);
  }
};

struct Queue {
  using TagVal = Tags::TagVal;
  std::deque<TagVal> queue_;

  void read(std::istream& is, const Tags& tags) {
    std::string pat, tagStr;
    is >> pat;
    checkPattern(pat, "initial:");
    std::getline(is, pat);
    auto iss = readLineToSS(is);
    while (iss >> tagStr) {
      auto& tag = tags.getTag(tagStr);
      queue_.emplace_back(tag.val_);
    }
  }

  void dump(std::ostream& os, const Tags& tags) const {
    for (auto && tagVal : queue_) {
      os << tags.getTagStr(tagVal) << " ";
    }
    os << "\n";
  }
};

} // namespace detail

class TagSystem {
  using Tags = detail::Tags;
  using Queue = detail::Queue;
  using TagVal = Tags::TagVal;
  
  Tags tags_;
  Queue queue_;

public:
  void read(std::istream& is) {
    tags_.read(is);
    queue_.read(is, tags_);
  }

  void dumpTable(std::ostream& os) const {
    tags_.dump(os);
  }

  void dumpState(std::ostream& os) const {
    queue_.dump(os, tags_);
  }

  void step() {
    auto& queue = queue_.queue_;
    auto& tag = tags_.getTag(queue.front());
    auto& app = tag.append_;
    std::move(app.begin(), app.end(), std::back_inserter(queue));
    queue.pop_front();
    queue.pop_front();
  }

  bool onHead() const {
    auto head = queue_.queue_.front();
    auto headStr = tags_.getTagStr(head);
    auto fir = *headStr.begin(), last = *std::prev(headStr.end());
    return fir == 'H' && (last == '0' || last == '1');
  }

  bool hlt() const {
    return tags_.hlt(queue_.queue_.front());
  }
};

} // namespace machines
