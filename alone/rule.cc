#include "alone/rule.hh"

#include "alone/context.hh"
#include "alone/final.hh"

namespace alone {

void Rule::FinishedAdding(const Context &context, search::Score additive) {
  additive_ = additive;
  search::Score lm_score = 0.0;
  lm::ngram::ChartState ignored;
  for (std::vector<Word>::const_iterator word = items_.begin(); ; ++word) {
    lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), ignored);
    for (; ; ++word) {
      if (word == items_.end()) {
        bound_ = additive_ + context.GetWeights().LMWeight() * lm_score;
        return;
      }
      if (!word->Terminal()) break;
      scorer.Terminal(word->Index());
    }
    lm_score += scorer.Finish();
  }
}

search::Score Rule::Apply(const Context &context, const std::vector<const Final *> &children, lm::ngram::ChartState &state) const {
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  std::vector<const Final*>::const_iterator child(children.begin());
  search::Score ret = additive_;
  for (std::vector<Word>::const_iterator i = items_.begin(); i != items_.end(); ++i) {
    if (i->Terminal()) {
      scorer.Terminal(i->Index());
    } else {
      scorer.NonTerminal((*child)->State(), 0.0);
      ret += (*child)->Total();
    }
  }
  return ret + context.GetWeights().LMWeight() * scorer.Finish();
}

} // namespace alone
