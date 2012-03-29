#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "lm/left.hh"
#include "search/arity.hh"
#include "search/rule.hh"
#include "search/types.hh"
#include "search/vertex.hh"

namespace search {

class Edge {
  public:
    typedef Vertex Child;

  public:
    Edge() {
      end_to_ = to_;
    }

    Rule &InitRule() { return rule_; }

    void Add(Child &vertex) {
      assert(end_to_ - to_ < kMaxArity);
      *(end_to_++) = &vertex;
    }

  private:
    // Rule and pointers to rule arguments.  
    Rule rule_;

    Child *to_[kMaxArity];
    Child **end_to_;
};

} // namespace search
#endif // SEARCH_EDGE__
