#ifndef ALONE_CONFIG__
#define ALONE_CONFIG__

#include "alone/features.hh"
#include "search/config.hh"

namespace util { class FilePiece; }

namespace alone {

class Config {
  public:
    Config(util::FilePiece &weights_file, unsigned int pop_limit, unsigned int nbest) 
      : weights_(weights_file), search_(weights_.GetSearch(), pop_limit, search::NBestConfig(nbest)) {}

    const feature::Weights &GetWeights() const { return weights_; }

    feature::Weights &MutableWeights() { return weights_; }

    const search::Config &GetSearch() const { return search_; }

  private:
    feature::Weights weights_;

    search::Config search_;
};

} // namespace alone
#endif // ALONE_CONFIG__
