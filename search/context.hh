#ifndef SEARCH_CONTEXT__
#define SEARCH_CONTEXT__

#include "lm/model.hh"
#include "search/config.hh"
#include "search/final.hh"
#include "search/types.hh"
#include "search/vertex.hh"
#include "search/word.hh"
#include "util/exception.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace search {

class Weights;

class ContextBase {
  public:
    ContextBase(const Config &config, Word end_sentence) : pop_limit_(config.PopLimit()), end_sentence_(end_sentence), weights_(config.GetWeights()) {}

    Final *NewFinal() {
     Final *ret = final_pool_.construct();
     assert(ret);
     return ret;
    }

    VertexNode *NewVertexNode() {
      VertexNode *ret = vertex_node_pool_.construct();
      assert(ret);
      return ret;
    }

    void DeleteVertexNode(VertexNode *node) {
      vertex_node_pool_.destroy(node);
    }

    unsigned int PopLimit() const { return pop_limit_; }

    Word EndSentence() const { return end_sentence_; }

    const Weights &GetWeights() const { return weights_; }

  private:
    boost::object_pool<Final> final_pool_;
    boost::object_pool<VertexNode> vertex_node_pool_;

    unsigned int pop_limit_;

    Word end_sentence_;

    const Weights &weights_;

};

template <class Model> class Context : public ContextBase {
  public:
    Context(const Config &config, Word end_sentence, const Model &model) : ContextBase(config, end_sentence), model_(model) {}

    const Model &LanguageModel() const { return model_; }

  private:
    const Model &model_;
};

} // namespace search

#endif // SEARCH_CONTEXT__
