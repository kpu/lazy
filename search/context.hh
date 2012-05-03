#ifndef SEARCH_CONTEXT__
#define SEARCH_CONTEXT__

#include "lm/model.hh"
#include "search/config.hh"
#include "search/final.hh"
#include "search/types.hh"
#include "search/vertex.hh"
#include "util/exception.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace search {

class Weights;

class Context {
  public:
    explicit Context(const Config &config) : pop_limit_(config.PopLimit()), model_(config.LanguageModel()), vocab_(model_.BaseVocabulary()), weights_(config.GetWeights()) {}

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

    const lm::ngram::RestProbingModel &LanguageModel() const { return model_; }

    Vocab &MutableVocab() { return vocab_; }

    const Vocab &GetVocab() const { return vocab_; }

    const Weights &GetWeights() const { return weights_; }

  private:
    boost::object_pool<Final> final_pool_;
    boost::object_pool<VertexNode> vertex_node_pool_;

    unsigned int pop_limit_;

    const lm::ngram::RestProbingModel &model_;

    Vocab vocab_;

    const Weights &weights_;
};

} // namespace search

#endif // SEARCH_CONTEXT__
