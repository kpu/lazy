#ifndef ALONE_RULE__
#define ALONE_RULE__

#include "alone/vocab.hh"
#include "search/arity.hh"
#include "search/types.hh"

#include <boost/array.hpp>

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

    Rule() : variables_(0), bos_(false) {}

    void AppendTerminal(Word w) { items_.push_back(w); }

    void AppendNonTerminal() {
      items_.resize(items_.size() + 1);
      ++variables_;
    }

    void FinishedAdding(const Context &context, search::Score additive, bool bos);

    search::Score Bound() const { return bound_; }

    search::Index Variables() const { return variables_; }

    search::Score Apply(const Context &context, const boost::array<const Final*, search::kMaxArity> &children, lm::ngram::ChartState &state) const;

    // For printing.  
    typedef const std::vector<Word> ItemsRet;
    ItemsRet &Items() const { return items_; }

  private:
    search::Score bound_, additive_;

    search::Index variables_;

    // TODO: pool?
    std::vector<Word> items_;

    bool bos_;
};

} // namespace alone

#endif // ALONE_RULE__
