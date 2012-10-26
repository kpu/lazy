#ifndef ALONE_EDGE_WORDS__
#define ALONE_EDGE_WORDS__

#include <string>
#include <vector>

namespace alone {

class EdgeWords {
  public:
    EdgeWords() {}

    void AppendWord(const std::string *word) {
      words_.push_back(word);
    }

    const std::vector<const std::string *> &Words() const {
      return words_;
    }

  private:
    // NULL for non-terminals.  
    std::vector<const std::string*> words_;
};

} // namespace alone

#endif // ALONE_EDGE_WORDS__
