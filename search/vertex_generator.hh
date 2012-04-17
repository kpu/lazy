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
    Context &context_;

    std::vector<EdgeGenerator> edges_;

    struct LessByTop : public std::binary_function<const EdgeGenerator *, const EdgeGenerator *, bool> {
      bool operator()(const EdgeGenerator *first, const EdgeGenerator *second) const {
        return first->Top() < second->Top();
      }
    };

    typedef std::priority_queue<EdgeGenerator*, std::vector<EdgeGenerator*>, LessByTop> Generate;
    Generate generate_;

    struct Trie {
      VertexNode *under;
      boost::unordered_map<Word, Trie> extend;

      Trie &Advance(Word word);
    };

    Trie root_;
};

} // namespace search
#endif // SEARCH_VERTEX_GENERATOR__
