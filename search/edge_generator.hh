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

template <class Model> class Context;

class VertexGenerator;

class EdgeGenerator {
  public:
    // True if it has a hypothesis.  
    bool Init(Edge &edge);

    Score Top() const {
      return top_;
    }

    template <class Model> bool Pop(Context<Model> &context, VertexGenerator &parent);

  private:
    template <class Model> void RecomputeFinal(Context<Model> &context, const PartialEdge &to, lm::ngram::ChartState &state);

    template <class Model> Score Adjustment(Context<Model> &context, const PartialEdge &to) const;

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
