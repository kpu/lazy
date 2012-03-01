#include "alone/weights.hh"
#include "util/tokenize_piece.hh"

#include <cstdlib>

namespace alone {

namespace {
struct Insert {
  void operator()(boost::unordered_map<std::string, search::Score> &map, StringPiece name, search::Score score) const {
    std::string copy(name.data(), name.size());
    map[copy] = score;
  }
};

struct DotProduct {
  search::Score total;
  DotProduct() : total(0.0) {}

  void operator()(const boost::unordered_map<std::string, search::Score> &map, StringPiece name, search::Score score) {
    boost::unordered_map<std::string, search::Score>::const_iterator i(FindStringPiece(map, name));
    if (i == map.end()) return;
    total += score * i->second;
  }
};

template <class Map, class Op> void Parse(StringPiece text, Map &map, Op &op) {
  for (util::TokenIter<util::SingleCharacter, true> spaces(text, ' '); spaces; ++spaces) {
    util::TokenIter<util::SingleCharacter> equals(*spaces, '=');
    UTIL_THROW_IF(!equals, WeightParseException, "Bad weight token " << *spaces);
    StringPiece name(*equals);
    UTIL_THROW_IF(!++equals, WeightParseException, "Bad weight token " << *spaces);
    char *end;
    // Assumes proper termination.  
    double value = std::strtod(equals->data(), &end);
    UTIL_THROW_IF(end != equals->data() + equals->size(), WeightParseException, "Failed to parse weight" << *equals);
    UTIL_THROW_IF(++equals, WeightParseException, "Too many equals in " << *spaces);
    op(map, name, value);
  }
}

} // namespace

Weights::Weights(StringPiece text) {
  Insert op;
  Parse<Map, Insert>(text, map_, op);
  lm_weight_ = Steal("LanguageModel");
  oov_weight_ = Steal("OOV");
}

search::Score Weights::DotNoLM(StringPiece text) const {
  DotProduct dot;
  Parse<const Map, DotProduct>(text, map_, dot);
  return dot.total;
}

float Weights::Steal(const std::string &str) {
  Map::iterator i(map_.find(str));
  if (i == map_.end()) {
    return 0.0;
  } else {
    float ret = i->second;
    map_.erase(i);
    return ret;
  }
}

} // namespace alone
