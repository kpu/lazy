#ifndef SEARCH_EDGE_GENERATOR__
#define SEARCH_EDGE_GENERATOR__

#include "search/edge.hh"

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

    Score Top() const {
      return top_;
    }

    template <class Model> bool Pop(Context<Model> &context, VertexGenerator &parent);

  private:
    const Rule &GetRule() const {
      return from_->GetRule();
    }

    Score top_;

    typedef std::priority_queue<PartialEdge*, std::vector<PartialEdge*>, PartialEdgePointerLess> Generate;
    Generate generate_;

    Edge *from_;
};

} // namespace search
#endif // SEARCH_EDGE_GENERATOR__
