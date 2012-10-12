#include "search/rule.hh"

#include "search/context.hh"
#include "search/final.hh"

#include <ostream>

#include <math.h>

namespace search {

template <class Model> Rule::InitRet Rule::InternalInit(const Model &model, const std::vector<lm::WordIndex> &words, bool prepend_bos) {
  InitRet ret;
  ret.oov = 0;
  ret.prob = 0.0;
  lexical_.clear();
  const lm::WordIndex oov = model.GetVocabulary().NotFound();

  for (std::vector<lm::WordIndex>::const_iterator word = words.begin(); ; ++word) {
    lexical_.resize(lexical_.size() + 1);
    lm::ngram::RuleScore<Model> scorer(model, lexical_.back());
    // TODO: optimize
    if (prepend_bos && (word == words.begin())) {
      scorer.BeginSentence();
    }
    for (; ; ++word) {
      if (word == words.end()) {
        ret.prob += scorer.Finish();
        arity_ = lexical_.size() - 1; 
        return ret;
      }
      if (*word == kNonTerminal) break;
      if (*word == oov) ++ret.oov;
      scorer.Terminal(*word);
    }
    ret.prob += scorer.Finish();
  }
}

template Rule::InitRet Rule::InternalInit(const lm::ngram::RestProbingModel &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template Rule::InitRet Rule::InternalInit(const lm::ngram::ProbingModel &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template Rule::InitRet Rule::InternalInit(const lm::ngram::TrieModel &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template Rule::InitRet Rule::InternalInit(const lm::ngram::QuantTrieModel &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template Rule::InitRet Rule::InternalInit(const lm::ngram::ArrayTrieModel &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template Rule::InitRet Rule::InternalInit(const lm::ngram::QuantArrayTrieModel &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);

} // namespace search
