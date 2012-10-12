#ifndef SEARCH_RULE__
#define SEARCH_RULE__

#include "lm/left.hh"
#include "lm/word_index.hh"
#include "search/arity.hh"
#include "search/types.hh"

#include <boost/array.hpp>

#include <iosfwd>
#include <vector>

namespace search {

template <class Model> class Context;

const lm::WordIndex kNonTerminal = lm::kMaxWordIndex;

class Rule {
  public:
    Rule() : arity_(0) {}

    template <class Model> float Init(const Context<Model> &context, const std::vector<lm::WordIndex> &words, bool prepend_bos);

    unsigned int Arity() const { return arity_; }

    const lm::ngram::ChartState &Lexical(unsigned int index) const {
      return lexical_[index];
    }

  private:
    unsigned int arity_;

    std::vector<lm::ngram::ChartState> lexical_;
};

} // namespace search

#endif // SEARCH_RULE__
