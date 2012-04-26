#ifndef SEARCH_EDGE_GENERATOR__
#define SEARCH_EDGE_GENERATOR__

#include "search/edge.hh"

#include <boost/unordered_map.hpp>
#include <queue>

namespace lm {
namespace ngram {
class ChartState;
} // namespace ngram
} // namespace lm

namespace search {

class Context;

class VertexGenerator;

class EdgeGenerator {
  public:
    // True if it has a hypothesis.  
    bool Init(Edge &edge);

    Score Top() const {
      return top_;
    }

    bool Pop(Context &context, VertexGenerator &parent);

  private:
    unsigned int PickVictim(const PartialEdge &in) const;

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
#endif // SEARCH_EDGE_GENERATOR__
