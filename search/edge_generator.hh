#ifndef SEARCH_EDGE_GENERATOR__
#define SEARCH_EDGE_GENERATOR__

#include "search/edge.hh"

#include <boost/pool/pool.hpp>
#include <boost/unordered_map.hpp>

#include <functional>
#include <queue>

namespace lm {
namespace ngram {
class ChartState;
} // namespace ngram
} // namespace lm

namespace search {

template <class Model> class Context;

class VertexGenerator;

struct PartialEdgePointerLess : std::binary_function<const PartialEdge *, const PartialEdge *, bool> {
  bool operator()(const PartialEdge *first, const PartialEdge *second) const {
    return *first < *second;
  }
};

class EdgeGenerator {
  public:
    // Precondition: edge is non-empty: every vertex has at least one hypothesis.  
    // The root is uninitialized, but given as a parameter for memory management purposes.  
    EdgeGenerator(Edge &edge, PartialEdge &root);

    Score TopScore() const {
      return top_score_;
    }

    const Edge &GetEdge() const {
      return *from_;
    }

    // Pop.  If there's a complete hypothesis, return it.  Otherwise return NULL.  
    template <class Model> PartialEdge *Pop(Context<Model> &context, boost::pool<> &partial_edge_pool);

  private:
    const Rule &GetRule() const {
      return from_->GetRule();
    }

    Score top_score_;

    typedef std::priority_queue<PartialEdge*, std::vector<PartialEdge*>, PartialEdgePointerLess> Generate;
    Generate generate_;

    Edge *from_;
};

} // namespace search
#endif // SEARCH_EDGE_GENERATOR__
