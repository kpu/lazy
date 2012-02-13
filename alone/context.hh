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
    explicit Context(const char *file, StringPiece weights) : lm_(file), vocab_(lm_.BaseVocabulary()), weights_(weights) {}

    const lm::ngram::Model &LanguageModel() const { return lm_; }

    Vocab &MutableVocab() { return vocab_; }

    const Weights &GetWeights() const { return weights_; }

  private:
    lm::ngram::Model lm_;

    Vocab vocab_;

    Weights weights_;
};

} // namespace alone
#endif // ALONE_CONTEXT__
