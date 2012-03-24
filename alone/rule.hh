#ifndef ALONE_RULE__
#define ALONE_RULE__

#include "alone/vocab.hh"
#include "search/arity.hh"
#include "search/types.hh"

#include <boost/array.hpp>

#include <iosfwd>
#include <vector>

namespace lm { namespace ngram {
  class ChartState;
} } // namespace lm

namespace alone {

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

    void MiddleState(const Context &context, lm::ngram::ChartState &to);

    // For printing.  
    typedef const std::vector<Word> ItemsRet;
    ItemsRet &Items() const { return items_; }

  private:
    search::Score bound_, additive_;

    search::Index arity_;

    // TODO: pool?
    std::vector<Word> items_;

    bool bos_;
};

std::ostream &operator<<(std::ostream &o, const Rule &rule);

} // namespace alone

#endif // ALONE_RULE__
