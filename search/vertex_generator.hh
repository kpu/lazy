#ifndef SEARCH_VERTEX_GENERATOR__
#define SEARCH_VERTEX_GENERATOR__

#include "search/edge_generator.hh"

#include <boost/unordered_map.hpp>

#include <queue>

namespace lm {
namespace ngram {
class ChartState;
} // namespace ngram
} // namespace lm

namespace search {

class Context;

class VertexGenerator {
  public:
    VertexGenerator(Context &context, Vertex &gen);

    void NewHypothesis(const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial);

  private:
    // Parallel structure to VertexNode.  
    struct Trie {
      Trie() : under(NULL) {}

      VertexNode *under;
      boost::unordered_map<uint64_t, Trie> extend;
    };

    Trie &FindOrInsert(Trie &node, uint64_t added, const lm::ngram::ChartState &state, unsigned char left, unsigned char right);

    void CompleteTransition(Trie &node, const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial);

    Context &context_;

    std::vector<EdgeGenerator> edges_;

    struct LessByTop : public std::binary_function<const EdgeGenerator *, const EdgeGenerator *, bool> {
      bool operator()(const EdgeGenerator *first, const EdgeGenerator *second) const {
        return first->Top() < second->Top();
      }
    };

    typedef std::priority_queue<EdgeGenerator*, std::vector<EdgeGenerator*>, LessByTop> Generate;
    Generate generate_;

    Trie root_;

    int to_pop_;
};

} // namespace search
#endif // SEARCH_VERTEX_GENERATOR__
