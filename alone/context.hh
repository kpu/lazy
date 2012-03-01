#ifndef ALONE_CONTEXT__
#define ALONE_CONTEXT__

#include "alone/final.hh"
#include "alone/vocab.hh"
#include "alone/weights.hh"
#include "lm/model.hh"
#include "search/context.hh"
#include "util/string_piece.hh"

namespace alone {

class Context : public search::Context<Final> {
  public:
    explicit Context(const lm::ngram::RestProbingModel &lm, Weights weights) : lm_(lm), vocab_(lm.BaseVocabulary()), weights_(weights) {}

    const lm::ngram::RestProbingModel &LanguageModel() const { return lm_; }

    Vocab &MutableVocab() { return vocab_; }

    const Vocab &GetVocab() const { return vocab_; }

    const Weights &GetWeights() const { return weights_; }

  private:
    const lm::ngram::RestProbingModel &lm_;

    Vocab vocab_;

    Weights weights_;
};

} // namespace alone
#endif // ALONE_CONTEXT__
