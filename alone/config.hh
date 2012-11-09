#ifndef ALONE_CONFIG__
#define ALONE_CONFIG__

#include "alone/weights.hh"
#include "search/config.hh"

namespace alone {

class Config {
  public:
    Config(StringPiece weight_str, unsigned int pop_limit, unsigned int nbest) 
      : weights_(weight_str), search_(weights_, pop_limit, search::NBestConfig(nbest)) {}

    const Weights &GetWeights() const { return weights_; }

    const search::Config &GetSearch() const { return search_; }

  private:
    Weights weights_;

    search::Config search_;
};

} // namespace alone
#endif // ALONE_CONFIG__
