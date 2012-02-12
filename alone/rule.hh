#ifndef ALONE_RULE__
#define ALONE_RULE__

#include "alone/final.hh"
#include "alone/vocab.hh"
#include "search/types.hh"

#include <vector>

namespace alone {

class Rule {
  public:
    typedef ::alone::Final Final;

    search::Score Bound() const { return bound_; }
    // Additive part of the score (i.e. excluding the language model).
    search::Score Additive() const { return additive_; }

    search::Index Variables() const { return variables_; }

    const std::vector<Word> &Items() const { return items_; }

  private:
    search::Score bound_, additive_;

    search::Index variables_;

    std::vector<Word> items_;
};

} // namespace alone

#endif // ALONE_RULE__
