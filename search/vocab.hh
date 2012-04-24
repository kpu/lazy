#ifndef SEARCH_VOCAB__
#define SEARCH_VOCAB__

#include "lm/word_index.hh"
#include "search/word.hh"
#include "util/string_piece.hh"

#include <boost/functional/hash/hash.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace lm { namespace base { class Vocabulary; } }

namespace search {

class Vocab {
  public:
    explicit Vocab(const lm::base::Vocabulary &backing);

    Word FindOrAdd(const StringPiece &str);

    Word EndSentence() const {
      return end_sentence_;
    }

  private:
    typedef boost::unordered_map<std::string, lm::WordIndex> Map;
    Map map_;

    const lm::base::Vocabulary &backing_;

    Word end_sentence_;
};

} // namespace search
#endif // SEARCH_VCOAB__