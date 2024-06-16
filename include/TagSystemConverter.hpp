#pragma once

#include "TagSystem.hpp"

namespace machines {

class TSConverter final : public Converter<TSConverter, ts::TagSystem> {
  using TagSystem = ts::TagSystem;
  using Tags = ts::Tags;
  using Queue = ts::Queue;
  using TagIndx = Tags::TagIndx;

  std::string tagToString(TagIndx val, size_t tagsCount) const {
    std::string str(tagsCount, 'N');
    str.at(val) = 'Y';
    return str;
  }

  void writeTable(const TagSystem &ts, std::ostream &os) const {
    auto &&tags = ts.tags_;
    auto tagsCount = tags.size();
    os << "table:\n";
    for (auto &&tag : tags) {
      if (tag.append_.empty()) {
        os << "- ";
      } else {
        for (auto &&append : tag.append_) {
          os << tagToString(append, tagsCount);
        }
      }
      os << " ";
    }
    for (size_t i = 0; i != tagsCount; ++i) {
      os << "- ";
    }
    os << "\n\n";
  }

  void writeHalt(const TagSystem &ts, std::ostream &os) const {
    auto &&tags = ts.tags_;
    auto tagsCount = tags.size();
    os << "halt:\n";
    for (auto halt = tags.haltBegin(); halt != tags.haltEnd(); ++halt) {
      os << tagToString(*halt, tagsCount) << " ";
    }
    os << "\n\n";
  }

  void writeInitial(const TagSystem &ts, std::ostream &os) const {
    auto &&queue = ts.queue_;
    auto &&tags = ts.tags_;
    auto tagsCount = tags.size();
    os << "initial:\n";
    for (auto &&tag : queue)
      os << tagToString(tag, tagsCount);
    os << "\n";
  }

public:
  void convert(std::istream &is, std::ostream &os) {
    TagSystem ts;
    ts.read(is);
    writeTable(ts, os);
    writeHalt(ts, os);
    writeInitial(ts, os);
  }
};

} // namespace machines
