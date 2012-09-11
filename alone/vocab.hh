#ifndef ALONE_VOCAB__
#define ALONE_VOCAB__

#include "lm/word_index.hh"
#include "search/word.hh"
#include "util/string_piece.hh"

#include <boost/functional/hash/hash.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace lm { namespace base { class Vocabulary; } }

namespace alone {

class Vocab {
  public:
    explicit Vocab(const lm::base::Vocabulary &backing);

    search::Word FindOrAdd(const StringPiece &str);

    search::Word EndSentence() const {
      return end_sentence_;
    }

  private:
    typedef boost::unordered_map<std::string, lm::WordIndex> Map;
    Map map_;

    const lm::base::Vocabulary &backing_;

    search::Word end_sentence_;
};

} // namespace alone
#endif // ALONE_VCOAB__
