#ifndef SEARCH_VERTEX__
#define SEARCH_VERTEX__

#include "lm/left.hh"
#include "search/edge.hh"
#include "search/final.hh"
#include "search/types.hh"

#include <boost/unordered_set.hpp>

#include <queue>
#include <vector>

#include <stdint.h>

namespace search {

class Edge;

class VertexNode {
  public:
    bool Complete() const {
      return extend_.empty();
    }

    Score Bound() const {
      return bound_;
    }

    const lm::ngram::ChartState &State() const { return state_; }

    Score Bound() const {
      return Complete() ? back_->Bound() : extend_.front().Bound();
    }

  private:
    std::vector<VectorNode> extend_;
    lm::ngram::ChartState state_;
    Final *back_;
};

class PartialVertex {
  public:
    bool Complete();

    Score Bound();

    Index Length();

    bool Split(Context &context, PartialVertex &continuation, PartialVertex &alternate);
};

class Vertex {
  public:
    Vertex() 
#ifdef DEBUG
      : finished_adding_(false)
#endif
    {}

    void Add(Edge &edge) {
#ifdef DEBUG
      assert(!finished_adding_);
#endif
      edges_.push_back(&edge);
    }

    void FinishedAdding() {
#ifdef DEBUG
      assert(!finished_adding_);
      finished_adding_ = true;
#endif
    }

    const PartialVertex &RootPartial() const { return root_partial_; }

  private:
    std::vector<Edge*> edges_;

#ifdef DEBUG
    bool finished_adding_;
#endif


    PartialVertex root_partial_;
};

class VertexGenerator {
  public:
    VertexGenerator(Vertex &gen);

    void NewHypothesis(const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial);

  private:
    std::vector<EdgeGenerator> edges_;

    struct LessByTop : public std::binary_function<const EdgeGenerator *, const EdgeGenerator *, bool> {
      bool operator()(const EdgeGenerator *first, const EdgeGenerator *second) const {
        return first->Top() < second->Top();
      }
    };

    typedef std::priority_queue<EdgeGenerator*, std::vector<EdgeGenerator*>, LessByTop> Generate;
    Generate generate_;

    typedef boost::unordered_set<uint64_t> Dedupe;
    Dedupe dedupe_;
};

} // namespace search
#endif // SEARCH_VERTEX__
