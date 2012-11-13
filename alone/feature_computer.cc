#include "alone/feature_computer.hh"

#include <iostream>

namespace alone {
namespace feature {

const char *Computer::kLanguageModelName = "LanguageModel";

Computer::Computer(const Weights &process_weights) :
  weights_(Weights::ForThread(), process_weights),
  lm_(weights_.Lookup(kLanguageModelName, &std::cerr)),
  oov_(weights_.Lookup("OOV", &std::cerr)),
  word_penalty_(weights_.Lookup("WordPenalty", &std::cerr)) {
}

std::ostream &Computer::Breakdown::Print(std::ostream &o) const {
  return o << ' ' << kLanguageModelName << '=' << prob << " OOV=" << oov << " WordPenalty=" << word_penalty;
}

} // namespace feature
} // namespace alone
