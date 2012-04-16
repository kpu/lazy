#ifndef SEARCH_WORD__
#define SEARCH_WORD__

#include "lm/word_index.hh"

#include <string>
#include <utility>

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
    const std::pair<const std::string, lm::WordIndex> *entry_;
};

} // namespace search
#endif // SEARCH_WORD__
