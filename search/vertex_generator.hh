#ifndef SEARCH_VERTEX_GENERATOR__
#define SEARCH_VERTEX_GENERATOR__

#include "search/edge.hh"
#include "search/vertex.hh"

#include <boost/unordered_map.hpp>
#include <boost/version.hpp>

namespace lm {
namespace ngram {
class ChartState;
} // namespace ngram
} // namespace lm

namespace search {

class ContextBase;

#if BOOST_VERSION > 104200
// Parallel structure to VertexNode.  
struct Trie {
  Trie() : under(NULL) {}

  VertexNode *under;
  boost::unordered_map<uint64_t, Trie> extend;
};

void AddHypothesis(ContextBase &context, Trie &root, const NBestComplete &end);

#endif // BOOST_VERSION

// Output makes the single-best or n-best list.   
template <class Output> class VertexGenerator {
  public:
    VertexGenerator(ContextBase &context, Vertex &gen, Output &nbest) : context_(context), gen_(gen), nbest_(nbest) {
      gen.root_.InitRoot();
    }

    void NewHypothesis(PartialEdge partial) {
      nbest_.Add(existing_[hash_value(partial.CompletedState())], partial);
    }

    void FinishedSearch() {
#if BOOST_VERSION > 104200
      Trie root;
      root.under = &gen_.root_;
      for (typename Existing::const_iterator i(existing_.begin()); i != existing_.end(); ++i) {
        AddHypothesis(context_, root, nbest_.Complete(i->second));
      }
      root.under->SortAndSet(context_, NULL);
#else
      UTIL_THROW(util::Exception, "Upgrade Boost to >= 1.42.0 to use incremental search.");
#endif
    }

    const Vertex &Generating() const { return gen_; }

  private:
    ContextBase &context_;

    Vertex &gen_;

    typedef boost::unordered_map<uint64_t, typename Output::Combine> Existing;
    Existing existing_;

    Output &nbest_;
};

} // namespace search
#endif // SEARCH_VERTEX_GENERATOR__
