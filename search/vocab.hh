#ifndef SEARCH_VOCAB__
#define SEARCH_VOCAB__

#include "lm/word_index.hh"
#include "util/string_piece.hh"

#include <boost/functional/hash/hash.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace lm { namespace base { class Vocabulary; } }

namespace search {

class Word {
  public:
    // Construct a non-terminal.
    Word() : entry_(NULL) {}

    bool Terminal() const { return entry_ != NULL; }

    const std::string &String() const { return entry_->first; }

    lm::WordIndex Index() const { return entry_->second; }

  protected:
    friend class Vocab;
    explicit Word(const std::pair<const std::string, lm::WordIndex> &entry) {
      entry_ = &entry;
    }

  private:
    friend size_t hash_value(const Word &word);
    const std::pair<const std::string, lm::WordIndex> *entry_;
};

// Pointers are unique.  
size_t hash_value(const Word &word) {
  return boost::hash_value(word.entry_);
}

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
