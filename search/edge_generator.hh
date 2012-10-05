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

    Score Top() const {
      return top_score_;
    }

    const Edge &GetEdge() const {
      return *from_;
    }

    template <class Model> PartialEdge *Pop(Context<Model> &context, boost::pool<> &partial_edge_pool) {
      assert(!generate_.empty());
      PartialEdge &top = *generate_.top();
      generate_.pop();
      unsigned int victim = 0;
      unsigned char lowest_length = 255;
      for (unsigned int i = 0; i != GetRule().Arity(); ++i) {
        if (!top.nt[i].Complete() && top.nt[i].Length() < lowest_length) {
          lowest_length = top.nt[i].Length();
          victim = i;
        }
      }
      if (lowest_length == 255) {
        // All states report complete.  
        top.between[0].right = top.between[GetRule().Arity()].right;
        // Now top.between[0] is the full edge state.  
        top_score_ = generate_.empty() ? -kScoreInf : generate_.top()->score;
        return &top;
      }
      Split(context, partial_edge_pool, top, victim);
      return NULL;
    }

  private:
    const Rule &GetRule() const {
      return from_->GetRule();
    }

    template <class Model> void Split(Context<Model> &context, boost::pool<> &partial_edge_pool, PartialEdge &top, unsigned int victim);

    Score top_score_;

    typedef std::priority_queue<PartialEdge*, std::vector<PartialEdge*>, PartialEdgePointerLess> Generate;
    Generate generate_;

    Edge *from_;
};

} // namespace search
#endif // SEARCH_EDGE_GENERATOR__
