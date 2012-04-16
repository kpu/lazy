#ifndef SEARCH_FINAL__
#define SEARCH_FINAL__

#include "search/types.hh"

#include <boost/array.hpp>

namespace alone {

class Final {
  public:
    typedef boost::array<const Final*, search::kMaxArity> ChildArray;

    Final(Score bound, const Rule &from, const ChildArray &children) : bound_(bound), from_(from), children_(children) {}

    const ChildArray &Children() const { return children_; }

    search::Index ChildCount() const { return from_.Variables(); }

    const Rule &From() const { return from_; }

    Score Bound() const { return bound_; }

  private:
    Score bound_;

    const Rule &from_;

    ChildArray children_;
};

} // namespace alone

#endif // SEARCH_FINAL__
