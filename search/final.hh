#ifndef SEARCH_FINAL__
#define SEARCH_FINAL__

#include "lm/left.hh"
#include "search/arity.hh"
#include "search/rule.hh"
#include "search/types.hh"

#include <boost/array.hpp>
#include <vector>

namespace alone {

class Context;
class Rule;

class Final {
  public:
    typedef boost::array<const Final*, search::kMaxArity> ChildArray;

    Final(const Context &context, const Rule &from, const ChildArray &children) : from_(from), children_(children) {
      total_ = from.Apply(context, children_, lm_state_);
    }

    search::Score Total() const { return total_; }

    uint64_t RecombineHash() const { return hash_value(lm_state_); }

    void Recombine(Context &context, Final *with) const;

    const lm::ngram::ChartState &State() const { return lm_state_; }

    const ChildArray &Children() const { return children_; }

    search::Index ChildCount() const { return from_.Variables(); }

    const Rule &From() const { return from_; }

  private:
    search::Score total_;
    lm::ngram::ChartState lm_state_;
    const Rule &from_;

    ChildArray children_;
};

} // namespace alone

#endif // SEARCH_FINAL__
