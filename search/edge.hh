#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "lm/left.hh"
#include "search/arity.hh"
#include "search/operation.hh"
#include "search/types.hh"
#include "search/vertex.hh"

namespace search {

class Partial;

class PartialWatch {
  public:

  private:
    Partial *watching_;
    Score cached_;
};

template <class Rule> class Edge;

template <class Rule> class EdgeEntry {
  public:
    EdgeEntry() {}

    void DownOne
    // True: we actually have a hypothesis to back up the score.
    // False: score may go down.  
    bool Hard() const { return hard_; }

    Score GetScore() const {
      return score_;
    }

    bool operator<(const EdgeEntry<Rule> &other) const {
      return score_ < other.score_;
    }

  private:
    Score score_;

    RuleIndex left_index_, right_index_;

    bool hard_;

    Edge<Rule> *edge_;

    PartialWatch left_, right_;

    lm::ngram::ChartState middle_;
};

template <class Rule> class Edge {
  public:
    typedef Vertex<Edge> Child;

  public:
    Edge() {
      end_to_ = to_;
    }

    Rule &InitRule() { return rule_; }

    void Add(Child &vertex) {
      assert(end_to_ - to_ < kMaxArity);
      *(end_to_++) = &vertex;
    }

    template <class C> void FinishedAdding(const C &context) {
      assert(end_to_ - to_ == rule_.Arity());
      EdgeEntry entry;
      rule_.MiddleState(context, entry.MutableMiddle());
      
      // TODO: arity > 2
      if (rule_.Arity()) {
        entry.SetLeft(to_[0].RootWatch());
      }
      if (rule_.Arity() == 2) {
        entry.SetRight(to_[1].RootWatch());
      }
    }

  private:
    // Rule and pointers to rule arguments.  
    Rule rule_;

    Child *to_[kMaxArity];
    Child **end_to_;

    boost::unordered_set<uint64_t> 
};

} // namespace search
#endif // SEARCH_EDGE__
