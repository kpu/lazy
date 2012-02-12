#ifndef ALONE_VOCAB__
#define ALONE_VOCAB__

#include "lm/word_index.hh"
#include "util/string_piece.hh"

#include <boost/unordered_map.hpp>

#include <string>

namespace lm { namespace base { class Vocabulary; } }

namespace alone {

class Word {
  public:
    // Construct a non-terminal.
    Word() : entry_(NULL) {}

    bool Terminal() const { return entry_ != NULL; }

    const std::string &String() const { return entry_->first; }

    lm::WordIndex Index() const { return entry_->second; }

  protected:
    friend class Vocab;
    explicit Word(const std::pair<std::string, lm::WordIndex> &entry) {
      entry_ = &entry;
    }

  private:
    const std::pair<std::string, lm::WordIndex> *entry_;
};

class Vocab {
  public:
    Vocab(lm::base::Vocabulary &backing) : backing_(backing) {}

    Word FindOrAdd(const StringPiece &str);

  private:
    typedef boost::unordered_map<std::string, lm::WordIndex> Map;
    Map map_;

    lm::base::Vocabulary &backing_;
};

} // namespace alone
#endif // ALONE_VCOAB__
