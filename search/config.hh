#ifndef SEARCH_CONFIG__
#define SEARCH_CONFIG__

#include "lm/model.hh"
#include "search/weights.hh"
#include "util/string_piece.hh"

namespace search {

class Config {
  public:
    Config(const lm::ngram::RestProbingModel &lm, StringPiece weight_str, unsigned int pop_limit) :
      lm_(lm), weights_(weight_str), pop_limit_(pop_limit) {}

    const lm::ngram::RestProbingModel &LanguageModel() const { return lm_; }

    const Weights &GetWeights() const { return weights_; }

    unsigned int PopLimit() const { return pop_limit_; }

  private:
    const lm::ngram::RestProbingModel &lm_;
    search::Weights weights_;
    unsigned int pop_limit_;
};

} // namespace search

#endif // SEARCH_CONFIG__
