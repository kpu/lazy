#ifndef ALONE_VOCAB__
#define ALONE_VOCAB__

#include "lm/word_index.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/functional/hash/hash.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace lm { namespace base { class Vocabulary; } }

namespace alone {

class Vocab {
  public:
    explicit Vocab(const lm::base::Vocabulary &backing);

    typedef std::pair<const StringPiece, lm::WordIndex> Entry;

    const Entry &FindOrAdd(const StringPiece &str);

    const Entry &EndSentence() const { return end_sentence_; }

  private:
    util::Pool piece_backing_;

    typedef boost::unordered_map<StringPiece, lm::WordIndex> Map;
    Map map_;

    const lm::base::Vocabulary &backing_;

    const std::pair<const StringPiece, lm::WordIndex> &end_sentence_;
};

} // namespace alone
#endif // ALONE_VCOAB__
