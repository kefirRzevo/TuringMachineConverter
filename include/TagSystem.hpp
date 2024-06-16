#pragma once

#include <algorithm>
#include <deque>
#include <iostream>
#include <vector>

#include "AbstractMachine.hpp"
#include "Utils.hpp"

namespace machines {

class TSConverter;

namespace ts {

class Tags final {
public:
  using TagIndx = unsigned int;
  using TagIndxStorage = std::vector<TagIndx>;
  using TagIndxConstIter = TagIndxStorage::const_iterator;

  struct Tag final {
    TagIndx indx_;
    std::string name_;
    TagIndxStorage append_;
  };

  using TagStorage = std::vector<Tag>;
  using TagConstIter = TagStorage::const_iterator;

private:
  TagStorage tags_;
  TagIndxStorage halts_;

  TagIndx getTagIndx(std::string_view tagName) const {
    auto found = std::find_if(tags_.begin(), tags_.end(),
                              [&](auto &&tag) { return tag.name_ == tagName; });
    if (found == tags_.end()) {
      auto msg = "Can't find tag '" + std::string(tagName) + "'";
      throw std::runtime_error(msg);
    }
    return found->indx_;
  }

  void readTags(std::istream &is) {
    std::string pat, tagName;
    is >> pat;
    utils::checkPattern(pat, "tags:");
    std::getline(is, pat);
    auto iss = utils::readLineToSS(is);
    while (iss >> tagName) {
      tags_.emplace_back(Tag{});
      auto &tag = tags_.back();
      tag.indx_ = tags_.size() - 1;
      tag.name_ = tagName;
    }
  }

  void readHalt(std::istream &is) {
    std::string pat, tagName;
    is >> pat;
    utils::checkPattern(pat, "halt:");
    std::getline(is, pat);
    auto iss = utils::readLineToSS(is);
    while (iss >> tagName)
      halts_.emplace_back(getTagIndx(tagName));
    std::sort(halts_.begin(), halts_.end());
  }

  void readTable(std::istream &is) {
    std::string pat, tagName;
    is >> pat;
    utils::checkPattern(pat, "table:");
    std::getline(is, pat);
    assert(tags_.size() > halts_.size());
    for (size_t i = 0; i != tags_.size() - halts_.size(); ++i) {
      auto iss = utils::readLineToSS(is);
      iss >> tagName >> pat;
      utils::checkPattern(pat, "->");
      auto tagIndx = getTagIndx(tagName);
      auto &tag = tags_.at(tagIndx);
      while (iss >> tagName)
        tag.append_.emplace_back(getTagIndx(tagName));
    }
  }

  void dumpTags(std::ostream &os) const {
    os << "tags:\n";
    for (auto &&tag : tags_)
      os << tag.name_ << " ";
    os << "\n\n";
  }

  void dumpHalt(std::ostream &os) const {
    os << "halt:\n";
    for (auto &&haltIndx : halts_)
      os << tags_.at(haltIndx).name_ << " ";
    os << "\n\n";
  }

  void dumpTable(std::ostream &os) const {
    os << "table:\n";
    for (auto &&tag : tags_) {
      if (isHlt(tag.indx_))
        continue;
      os << tag.name_ << " -> ";
      for (auto &&append : tag.append_)
        os << tags_.at(append).name_ << " ";
      os << "\n";
    }
    os << "\n\n";
  }

public:
  void read(std::istream &is) {
    readTags(is);
    readHalt(is);
    readTable(is);
  }

  const Tag &getTag(TagIndx indx) const { return tags_.at(indx); }

  const Tag &getTag(std::string_view tagName) const {
    return tags_.at(getTagIndx(tagName));
  }

  bool isHlt(TagIndx indx) const {
    return std::binary_search(halts_.begin(), halts_.end(), indx);
  }

  void dump(std::ostream &os) const {
    dumpTags(os);
    dumpHalt(os);
    dumpTable(os);
  }

  size_t size() const noexcept { return tags_.size(); }

  size_t haltSize() const noexcept { return halts_.size(); }

  TagIndxConstIter haltBegin() const noexcept { return halts_.begin(); }

  TagIndxConstIter haltEnd() const noexcept { return halts_.end(); }

  TagConstIter begin() const noexcept { return tags_.begin(); }

  TagConstIter end() const noexcept { return tags_.end(); }
};

class Queue final {
  using TagIndx = Tags::TagIndx;
  using TagQueue = std::deque<TagIndx>;

  TagQueue queue_;

public:
  using TagQueueConstIter = TagQueue::const_iterator;
  using BackInsertIter = std::back_insert_iterator<TagQueue>;

  void read(std::istream &is, const Tags &tags) {
    std::string pat, tagName;
    is >> pat;
    utils::checkPattern(pat, "initial:");
    std::getline(is, pat);
    auto iss = utils::readLineToSS(is);
    while (iss >> tagName)
      queue_.emplace_back(tags.getTag(tagName).indx_);
  }

  void dump(std::ostream &os, const Tags &tags) const {
    for (auto &&tagIndx : queue_)
      os << tags.getTag(tagIndx).name_ << " ";
    os << "\n";
  }

  bool empty() const noexcept { return queue_.empty(); }

  void popTwoTags() noexcept {
    queue_.pop_front();
    queue_.pop_front();
  }

  TagIndx frontTagIndx() const noexcept { return queue_.front(); }

  BackInsertIter backInserter() { return std::back_inserter(queue_); }

  TagQueueConstIter begin() const noexcept { return queue_.begin(); }

  TagQueueConstIter end() const noexcept { return queue_.end(); }
};

class TagSystem final : public machines::Machine<TagSystem> {
  using TagIndx = Tags::TagIndx;

  Tags tags_;
  Queue queue_;

  void read(std::istream &is) {
    tags_.read(is);
    queue_.read(is, tags_);
  }

  void dumpTable(std::ostream &os) const { tags_.dump(os); }

  void dumpState(std::ostream &os) const { queue_.dump(os, tags_); }

  void step() {
    if (queue_.empty()) {
      auto msg = "Queue is empty";
      throw std::runtime_error(msg);
    }
    const auto &tag = tags_.getTag(queue_.frontTagIndx());
    const auto &append = tag.append_;
    std::copy(append.begin(), append.end(), queue_.backInserter());
    queue_.popTwoTags();
  }

  bool onHead() const {
    const auto &head = tags_.getTag(queue_.frontTagIndx());
    const auto &headName = head.name_;
    auto fir = *headName.begin(), last = *std::prev(headName.end());
    return fir == 'H' && (last == '0' || last == '1');
  }

  bool hlt() const { return tags_.isHlt(queue_.frontTagIndx()); }

  friend class machines::TSConverter;

public:
  void execute(std::istream &is, std::ostream &os, DumpLvl lvl) {
    read(is);
    dumpTable(os);
    for (;;) {
      if (hlt())
        break;
      if (lvl > 0)
        if (onHead() || lvl > 1)
          dumpState(os);
      step();
    }
    dumpState(os);
    os.flush();
  }
};

} // namespace ts

} // namespace machines
