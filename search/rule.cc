#include "search/rule.hh"

#include "lm/model.hh"
#include "search/context.hh"

#include <ostream>

#include <math.h>

namespace search {

template <class Model> float ScoreRule(const Context<Model> &context, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing) {
  unsigned int oov_count = 0;
  float prob = 0.0;
  const Model &model = context.LanguageModel();
  const lm::WordIndex oov = model.GetVocabulary().NotFound();
  lm::ngram::RuleScore<Model> scorer(model, *(writing++));
  if (prepend_bos) {
    scorer.BeginSentence();
  }
  for (std::vector<lm::WordIndex>::const_iterator word = words.begin(); ; ++word) {
    // TODO: optimize
    for (; ; ++word) {
      if (word == words.end()) {
        prob += scorer.Finish();
        return static_cast<float>(oov_count) * context.GetWeights().OOV() + prob * context.GetWeights().LM();
      }
      if (*word == kNonTerminal) break;
      if (*word == oov) ++oov_count;
      scorer.Terminal(*word);
    }
    prob += scorer.Finish();
    scorer.Reset();
  }
}

template float ScoreRule(const Context<lm::ngram::RestProbingModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing);
template float ScoreRule(const Context<lm::ngram::ProbingModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing);
template float ScoreRule(const Context<lm::ngram::TrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing);
template float ScoreRule(const Context<lm::ngram::QuantTrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing);
template float ScoreRule(const Context<lm::ngram::ArrayTrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing);
template float ScoreRule(const Context<lm::ngram::QuantArrayTrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos, lm::ngram::ChartState *writing);

} // namespace search
