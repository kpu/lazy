#include "alone/rule.hh"

#include "alone/context.hh"
#include "alone/final.hh"

#include <ostream>

#include <math.h>

namespace alone {

float PositiveBackoffs(const lm::ngram::ChartState &state) {
  float ret = 0.0;
  for (unsigned char i = 0; i < state.right.length; ++i) {
    if (state.right.backoff[i] > 0.0) ret+= state.right.backoff[i];
  }
  return ret;
}

void Rule::FinishedAdding(const Context &context, search::Score additive, bool add_sentence_bounds) {
  const float word_penalty = -1.0 / M_LN10 * context.GetWeights().WordPenalty();
  additive_ = additive;
  bos_ = add_sentence_bounds;
  if (add_sentence_bounds) {
    AppendTerminal(context.GetVocab().EndSentence());
    // Don't count </s> as a word for purposes of word penalty.   
    additive_ -= word_penalty;
  }
  search::Score lm_score = 0.0;
  lm::ngram::ChartState state;
  const lm::WordIndex oov = context.LanguageModel().GetVocabulary().NotFound();
  for (std::vector<Word>::const_iterator word = items_.begin(); ; ++word) {
    lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
    // TODO: optimize
    if (bos_ && (word == items_.begin())) {
      scorer.BeginSentence();
    }
    for (; ; ++word) {
      if (word == items_.end()) {
        bound_ = additive_ + context.GetWeights().LM() * lm_score;
        return;
      }
      if (!word->Terminal()) break;
      if (word->Index() == oov) additive_ += context.GetWeights().OOV();
      scorer.Terminal(word->Index());
      additive_ += word_penalty;
    }
    lm_score += scorer.Finish();
    lm_score += PositiveBackoffs(state);
  }
}

search::Score Rule::Apply(const Context &context, const Final::ChildArray &children, lm::ngram::ChartState &state) const {
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  if (bos_) scorer.BeginSentence();
  const Final *const *child = children.data();
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
  ret += context.GetWeights().LM() * lm_score;
  assert(bound_ + 0.001 >= ret);
  return ret;
}

std::ostream &operator<<(std::ostream &o, const Rule &rule) {
  const Rule::ItemsRet &items = rule.Items();
  for (Rule::ItemsRet::const_iterator i = items.begin(); i != items.end(); ++i) {
    if (i->Terminal()) {
      o << i->String() << ' ';
    } else {
      o << "[] ";
    }
  }
  return o;
}

} // namespace alone
