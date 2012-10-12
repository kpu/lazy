#include "search/rule.hh"

#include "search/context.hh"
#include "search/final.hh"

#include <ostream>

#include <math.h>

namespace search {

template <class Model> float Rule::Init(const Context<Model> &context, const std::vector<lm::WordIndex> &words, bool prepend_bos) {
  unsigned int oov_count = 0;
  float prob = 0.0;
  lexical_.clear();
  const Model &model = context.LanguageModel();
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
        prob += scorer.Finish();
        arity_ = lexical_.size() - 1;
        return static_cast<float>(oov_count) * context.GetWeights().OOV() + prob * context.GetWeights().LM();
      }
      if (*word == kNonTerminal) break;
      if (*word == oov) ++oov_count;
      scorer.Terminal(*word);
    }
    prob += scorer.Finish();
  }
}

template float Rule::Init(const Context<lm::ngram::RestProbingModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template float Rule::Init(const Context<lm::ngram::ProbingModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template float Rule::Init(const Context<lm::ngram::TrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template float Rule::Init(const Context<lm::ngram::QuantTrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template float Rule::Init(const Context<lm::ngram::ArrayTrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);
template float Rule::Init(const Context<lm::ngram::QuantArrayTrieModel> &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);

} // namespace search
