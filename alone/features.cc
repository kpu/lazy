#include "alone/features.hh"

#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

#include <limits>

#include <string.h>

namespace alone {
namespace feature {

void Adder::Finish(Vector &out_wrap) {
  typedef Vector::Entry Entry;
  std::vector<Entry> &out = out_wrap.values_;
  out.clear();

  ID previous = std::numeric_limits<ID>::max();
  while (!queue_.empty()) {
    Part top(queue_.top());
    queue_.pop();

    if (previous == top.cur->id) {
      out.back().score += top.cur->score;
    } else {
      out.push_back(*top.cur);
      previous = top.cur->id;
    }

    ++top.cur;
    if (top.cur != top.end) {
      queue_.push(top);
    }
  }
}

namespace {

struct ParseRet {
  StringPiece name;
  search::Score score;
};

// Parser allows = or space between feature name and value.  
ParseRet RawParse(util::TokenIter<util::AnyCharacter, true> &spaces) {
  ParseRet ret;
  util::TokenIter<util::SingleCharacter> equals(*spaces, '=');

  assert(equals); // spaces skipped empty.
  ret.name = *equals;
  StringPiece value;
  if (++equals) {
    value = *equals;
    UTIL_THROW_IF(++equals, WeightParseException, "Too many equals in " << *spaces);
  } else {
    value = *++spaces;
  }
  ++spaces;

  char *end;
  // Assumes proper termination :-(
  ret.score = std::strtod(value.data(), &end);
  UTIL_THROW_IF(end != value.data() + value.size(), WeightParseException, "Failed to parse weight" << value);
  return ret;
}

} // namespace

WeightsBase::WeightsBase(util::FilePiece &f) {
  StringPiece line;
  while (true) {
    try {
      line = f.ReadLine();
    } catch (const util::EndOfFileException &e) {
      break;
    }
    for (util::TokenIter<util::AnyCharacter, true> spaces(line, " \n\t"); spaces;) {
      ParseRet res(RawParse(spaces));
      Add(res.name, res.score);
    }
  }
}

WeightsBase::WeightsBase(StringPiece from) {
  for (util::TokenIter<util::AnyCharacter, true> spaces(from, " \n\t"); spaces;) {
    ParseRet res(RawParse(spaces));
    Add(res.name, res.score);
  }
}

search::Score WeightsBase::Parse(StringPiece from, Vector &to) {
  to.values_.clear();
  search::Score dot = 0.0;
  Vector::Entry to_add;

  for (util::TokenIter<util::AnyCharacter, true> spaces(from, " \n\t"); spaces;) {
    ParseRet res(RawParse(spaces));
    to_add.score = res.score;
    Map::iterator i(str_.find(res.name));
    if (i != str_.end()) {
      dot += i->second.weight * res.score;
      to_add.id = i->second.id;
    } else {
      to_add.id = Add(res.name, 0.0);
    }
    to.values_.push_back(to_add);
  }
  std::sort(to.values_.begin(), to.values_.end());
  return dot;
}

search::Score WeightsBase::Lookup(StringPiece name, std::ostream *complain) const {
  Map::const_iterator i = str_.find(name);
  if (i == str_.end()) {
    if (complain) *complain << "Warning: hard-coded feature " << name << " was not given a weight." << std::endl;
    return 0.0;
  }
  return i->second.weight;
}

ID WeightsBase::Add(StringPiece str, search::Score weight) {
  char *copied_mem = static_cast<char*>(pool_.Allocate(str.size()));
  memcpy(copied_mem, str.data(), str.size());
  std::pair<StringPiece, Value> to_ins;
  to_ins.first = StringPiece(copied_mem, str.size());
  UTIL_THROW_IF(id_.size() > std::numeric_limits<ID>::max(), WeightParseException, "Too many features.  Change the typedef for ID to uint64_t.");
  to_ins.second.id = static_cast<ID>(id_.size());
  to_ins.second.weight = weight;
  std::pair<Map::iterator, bool> ret(str_.insert(to_ins));
  UTIL_THROW_IF(!ret.second, WeightParseException, "Duplicate identifier " << str);
  id_.push_back(to_ins.first);
  return to_ins.second.id;
}

std::ostream &WeightsBase::Write(std::ostream &to, const Vector &from) const {
  std::vector<Vector::Entry>::const_iterator i(from.values_.begin());
  if (i == from.values_.end()) return to;
  to << id_[i->id] << '=' << i->score;
  for (++i; i != from.values_.end(); ++i) 
    to << ' ' << id_[i->id] << '=' << i->score;
  return to;
}

Weights::Weights(util::FilePiece &f, std::ostream *complain) : 
  WeightsBase(f),
  lm_(Lookup("LanguageModel", complain)),
  oov_(Lookup("OOV", complain)),
  word_penalty_(Lookup("WordPenalty", complain)) {}

Weights::Weights(StringPiece str, std::ostream *complain) : 
  WeightsBase(str),
  lm_(Lookup("LanguageModel", complain)),
  oov_(Lookup("OOV", complain)),
  word_penalty_(Lookup("WordPenalty", complain)) {}

} // namespace feature
} // namespace alone
