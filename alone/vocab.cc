#include "alone/vocab.hh"

#include "lm/virtual_interface.hh"
#include "util/string_piece.hh"

namespace alone {

Vocab::Vocab(const lm::base::Vocabulary &backing) : backing_(backing), end_sentence_(FindOrAdd("</s>")) {}

const std::pair<const StringPiece, lm::WordIndex> &Vocab::FindOrAdd(const StringPiece &str) {
  Map::const_iterator i(map_.find(str));
  if (i != map_.end()) return *i;
  char *copied = static_cast<char*>(piece_backing_.Allocate(str.size()));
  memcpy(copied, str.data(), str.size());
  std::pair<StringPiece, lm::WordIndex> to_ins(StringPiece(copied, str.size()), backing_.Index(str));
  return *map_.insert(to_ins).first;
}

} // namespace alone
