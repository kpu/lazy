#include "search/rule.hh"

#include "search/context.hh"
#include "search/final.hh"

#include <ostream>

#include <math.h>

namespace search {

/*float PositiveBackoffs(const lm::ngram::ChartState &state) {
  float ret = 0.0;
  for (unsigned char i = 0; i < state.right.length; ++i) {
    if (state.right.backoff[i] > 0.0) ret+= state.right.backoff[i];
  }
  return ret;
}*/

template <class Model> void Rule::FinishedAdding(const Context<Model> &context, Score additive, bool add_sentence_bounds) {
  const float word_penalty = -1.0 / M_LN10 * context.GetWeights().WordPenalty();
  additive_ = additive;
  bos_ = add_sentence_bounds;
  if (add_sentence_bounds) {
    AppendTerminal(context.EndSentence());
    // Don't count </s> as a word for purposes of word penalty.   
    additive_ -= word_penalty;
  }
  Score lm_score = 0.0;
  lexical_.clear();
  const lm::WordIndex oov = context.LanguageModel().GetVocabulary().NotFound();

  for (std::vector<Word>::const_iterator word = items_.begin(); ; ++word) {
    lexical_.resize(lexical_.size() + 1);
    lm::ngram::RuleScore<Model> scorer(context.LanguageModel(), lexical_.back());
    // TODO: optimize
    if (bos_ && (word == items_.begin())) {
      scorer.BeginSentence();
    }
    for (; ; ++word) {
      if (word == items_.end()) {
        lm_score += scorer.Finish();
        bound_ = additive_ + context.GetWeights().LM() * lm_score;
        assert(lexical_.size() == arity_ + 1);
        return;
      }
      if (!word->Terminal()) break;
      if (word->Index() == oov) additive_ += context.GetWeights().OOV();
      scorer.Terminal(word->Index());
      additive_ += word_penalty;
    }
    lm_score += scorer.Finish();
//    lm_score += PositiveBackoffs(state);
  }
}

template void Rule::FinishedAdding(const Context<lm::ngram::RestProbingModel> &context, Score additive, bool add_sentence_bounds);
template void Rule::FinishedAdding(const Context<lm::ngram::ProbingModel> &context, Score additive, bool add_sentence_bounds);

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

} // namespace search
