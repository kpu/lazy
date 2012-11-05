#ifndef SEARCH_CONFIG__
#define SEARCH_CONFIG__

#include "search/weights.hh"

namespace search {

struct NBestConfig {
  explicit NBestConfig(unsigned int in_size) {
    keep = in_size;
    size = in_size;
  }
  
  unsigned int keep, size;
};

class Config {
  public:
    Config(const Weights &weights, unsigned int pop_limit, const NBestConfig &nbest) :
      weights_(weights), pop_limit_(pop_limit), nbest_(nbest) {}

    const Weights &GetWeights() const { return weights_; }

    unsigned int PopLimit() const { return pop_limit_; }

    const NBestConfig &GetNBest() const { return nbest_; }

  private:
    Weights weights_;
    unsigned int pop_limit_;

    NBestConfig nbest_;
};

} // namespace search

#endif // SEARCH_CONFIG__
