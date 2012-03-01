#include "alone/rule.hh"

#include "alone/context.hh"
#include "alone/final.hh"

namespace alone {

float PositiveBackoffs(const lm::ngram::ChartState &state) {
  float ret = 0.0;
  for (unsigned char i = 0; i < state.right.length; ++i) {
    if (state.right.backoff[i] > 0.0) ret+= state.right.backoff[i];
  }
  return ret;
}

void Rule::FinishedAdding(const Context &context, search::Score additive, bool bos) {
  additive_ = additive;
  bos_ = bos;
  search::Score lm_score = 0.0;
  lm::ngram::ChartState state;
  const lm::WordIndex oov = context.LanguageModel().GetVocabulary().NotFound();
  for (std::vector<Word>::const_iterator word = items_.begin(); ; ++word) {
    lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
    for (; ; ++word) {
      if (word == items_.end()) {
        bound_ = additive_ + context.GetWeights().LM() * lm_score;
        return;
      }
      if (!word->Terminal()) break;
      if (word->Index() == oov) additive_ += context.GetWeights().OOV();
      scorer.Terminal(word->Index());
    }
    lm_score += scorer.Finish();
    lm_score += PositiveBackoffs(state);
  }
}

search::Score Rule::Apply(const Context &context, const std::vector<const Final *> &children, lm::ngram::ChartState &state) const {
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  if (bos_) scorer.BeginSentence();
  std::vector<const Final*>::const_iterator child(children.begin());
  search::Score ret = additive_;
  for (std::vector<Word>::const_iterator i = items_.begin(); i != items_.end(); ++i) {
    if (i->Terminal()) {
      scorer.Terminal(i->Index());
    } else {
      scorer.NonTerminal((*child)->State(), -PositiveBackoffs((*child)->State()));
      ret += (*child)->Total();
      ++child;
    }
  }
  float lm_score = scorer.Finish();
  lm_score += PositiveBackoffs(state);
  return ret + context.GetWeights().LM() * lm_score;
}

} // namespace alone
