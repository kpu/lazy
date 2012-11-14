#include "alone/feature_computer.hh"

#include <iostream>

namespace alone {
namespace feature {

const char *Computer::kLanguageModelName = "LanguageModel";
const char *Computer::kOOVName = "LanguageModel_OOV";
const char *Computer::kWordPenaltyName = "WordPenalty";

void Computer::CheckForWeights(const Weights &in) {
  in.Lookup(kLanguageModelName, &std::cerr);
  in.Lookup(kOOVName, &std::cerr);
  in.Lookup(kWordPenaltyName, &std::cerr);
}

Computer::Computer(const Weights &process_weights) :
  weights_(Weights::ForThread(), process_weights),
  lm_(weights_.Lookup(kLanguageModelName)),
  oov_(weights_.Lookup(kOOVName)),
  word_penalty_(weights_.Lookup(kWordPenaltyName)) {
}

std::ostream &Computer::Breakdown::Print(std::ostream &o) const {
  return o << ' ' << kLanguageModelName << '=' << prob << ' ' << kOOVName << '=' << oov << ' ' << kWordPenaltyName << '=' << word_penalty;
}

} // namespace feature
} // namespace alone
