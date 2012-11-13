#include "search/rule.hh"

#include "lm/model.hh"
#include "search/context.hh"

#include <ostream>

#include <math.h>

namespace search {

template <class Model> ScoreRuleRet ScoreRule(const Model &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing) {
  ScoreRuleRet ret;
  ret.prob = 0.0;
  ret.oov = 0;
  const lm::WordIndex oov = model.GetVocabulary().NotFound(), bos = model.GetVocabulary().BeginSentence();
  for (std::vector<lm::WordIndex>::const_iterator word = words.begin(); ; ++word) {
    lm::ngram::RuleScore<Model> scorer(model, *(writing++));
    // TODO: optimize
    if (*word == bos) {
      scorer.BeginSentence();
      ++word;
    }
    for (; ; ++word) {
      if (word == words.end()) {
        ret.prob += scorer.Finish();
        return ret;
      }
      if (*word == kNonTerminal) break;
      if (*word == oov) ++ret.oov;
      scorer.Terminal(*word);
    }
    ret.prob += scorer.Finish();
  }
}

template ScoreRuleRet ScoreRule(const lm::ngram::RestProbingModel &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing);
template ScoreRuleRet ScoreRule(const lm::ngram::ProbingModel &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing);
template ScoreRuleRet ScoreRule(const lm::ngram::TrieModel &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing);
template ScoreRuleRet ScoreRule(const lm::ngram::QuantTrieModel &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing);
template ScoreRuleRet ScoreRule(const lm::ngram::ArrayTrieModel &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing);
template ScoreRuleRet ScoreRule(const lm::ngram::QuantArrayTrieModel &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *writing);

} // namespace search
