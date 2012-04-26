#ifndef SEARCH_CONTEXT__
#define SEARCH_CONTEXT__

#include "lm/model.hh"
#include "search/final.hh"
#include "search/types.hh"
#include "search/vertex.hh"
#include "search/weights.hh"
#include "util/exception.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace search {

class PoolOut : public util::Exception {
  public:
    PoolOut() throw() {
      *this << "Pool returned NULL";
    }

    ~PoolOut() throw() {}
};

class Context {
  public:
    Context(const lm::ngram::RestProbingModel &model, Weights weights, unsigned int pop_limit) : pop_limit_(pop_limit), model_(model), vocab_(model.BaseVocabulary()), weights_(weights) {}

    Final *NewFinal() {
     Final *ret = final_pool_.construct();
     if (!ret) throw PoolOut();
     return ret;
    }

    VertexNode *NewVertexNode() {
      VertexNode *ret = vertex_node_pool_.construct();
      if (!ret) throw PoolOut();
      return ret;
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

    Weights weights_;
};

} // namespace search

#endif // SEARCH_CONTEXT__
