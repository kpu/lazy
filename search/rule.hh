#ifndef SEARCH_RULE__
#define SEARCH_RULE__

#include "search/vocab.hh"
#include "search/arity.hh"
#include "search/types.hh"

#include <boost/array.hpp>

#include <iosfwd>
#include <vector>

namespace lm { namespace ngram {
  class ChartState;
} } // namespace lm

namespace search {

class Final;
class Context;

class Rule {
  public:
    typedef ::alone::Final Final;

    Rule() : arity_(0), bos_(false) {}

    void AppendTerminal(Word w) { items_.push_back(w); }

    void AppendNonTerminal() {
      items_.resize(items_.size() + 1);
      ++arity_;
    }

    void FinishedAdding(const Context &context, search::Score additive, bool add_sentence_bounds);

    search::Score Bound() const { return bound_; }

    search::Score Additive() const { return additive_; }

    search::Index Arity() const { return arity_; }

    bool BeginSentence() const { return bos_; }

    const lm::ngram::ChartState &Lexical(search::Index index) const {
      return lexical_[index];
    }

    // For printing.  
    typedef const std::vector<Word> ItemsRet;
    ItemsRet &Items() const { return items_; }

  private:
    search::Score bound_, additive_;

    search::Index arity_;

    // TODO: pool?
    std::vector<Word> items_;

    std::vector<lm::ngram::ChartState> lexical_;

    bool bos_;
};

std::ostream &operator<<(std::ostream &o, const Rule &rule);

} // namespace search

#endif // SEARCH_RULE__
