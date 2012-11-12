// search::PartialEdge has everything needed for decoding.  This has 
// information useful for output: the actual strings and feature values.

#ifndef ALONE_EDGE__
#define ALONE_EDGE__

#include "alone/features.hh"

#include <string>
#include <vector>

namespace alone {

class Edge {
  public:
    Edge() {}

    void AppendWord(const std::string *word) {
      words_.push_back(word);
    }

    const std::vector<const std::string *> &Words() const {
      return words_;
    }

    feature::Vector &InitFeatures() { return features_; }

    const feature::Vector &Features() const { return features_; }

  private:
    // NULL for non-terminals.  
    std::vector<const std::string*> words_;

    feature::Vector features_;
};

} // namespace alone

#endif // ALONE_EDGE__
