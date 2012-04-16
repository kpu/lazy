#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/arity.hh"
#include "search/rule.hh"
#include "search/types.hh"

#include <queue>

namespace search {

class Context;
class VertexGenerator;

class Edge {
  public:
    Edge() {
      end_to_ = to_;
    }

    Rule &InitRule() { return rule_; }

    void Add(Vertex &vertex) {
      assert(end_to_ - to_ < kMaxArity);
      *(end_to_++) = &vertex;
    }

    const Vertex &GetVertex(Index index) const {
      return to_[index];
    }

    const Rule &GetRule() const { return rule_; }

  private:
    // Rule and pointers to rule arguments.  
    Rule rule_;

    Vertex *to_[kMaxArity];
    Vertex **end_to_;
};

struct PartialEdge {
  Score score;
  PartialVertex nt[kMaxArity];
};

class EdgeGenerator {
  public:
    void Init(Edge &edge);

    Score Top() const {
      return top_;
    }

    void Pop(Context &context, VertexGenerator &parent);

  private:
    unsigned int PickVictim(const PartialEdge &in) const {
      // TODO: better decision rule.
      return in.nt[0].Length() >= in.nt[1].Length();
    }

    void RecomputeFinal(Context &context, const PartialEdge &to, lm::ngram::ChartState &state);

    Score Adjustment(Context &context, const PartialEdge &to) const;

    const Rule &GetRule() const {
      return from_->GetRule();
    }

    Score top_;

    typedef std::priority_queue<PartialEdge> Generate;
    Generate generate_;

    Edge *from_;
};

} // namespace search
#endif // SEARCH_EDGE__
