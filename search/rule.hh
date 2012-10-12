#ifndef SEARCH_RULE__
#define SEARCH_RULE__

#include "lm/left.hh"
#include "lm/word_index.hh"
#include "search/arity.hh"
#include "search/types.hh"

#include <boost/array.hpp>

#include <iosfwd>
#include <vector>

namespace search {

template <class Model> class Context;

const lm::WordIndex kNonTerminal = lm::kMaxWordIndex;

class Rule {
  public:
    Rule() : arity_(0) {}

    // Use kNonTerminal for non-terminals.  
    template <class C> void Init(const C &context, Score additive, const std::vector<lm::WordIndex> &words, bool prepend_bos) {
      InitRet ret(InternalInit(context.LanguageModel(), words, prepend_bos));
      bound_ = additive + static_cast<float>(ret.oov) * context.GetWeights().OOV();
      bound_ += ret.prob * context.GetWeights().LM();
    }

    template <class C> void InitFromTotal(const C &context, Score total, const std::vector<lm::WordIndex> &words, bool prepend_bos) {
      bound_ = total;
      InternalInit(context.LanguageModel(), words, prepend_bos);
    }

    Score Bound() const { return bound_; }

    unsigned int Arity() const { return arity_; }

    const lm::ngram::ChartState &Lexical(unsigned int index) const {
      return lexical_[index];
    }

  private:
    struct InitRet {
      unsigned int oov;
      float prob;
    };
    template <class Model> InitRet InternalInit(const Model &model, const std::vector<lm::WordIndex> &words, bool prepend_bos);

    Score bound_;

    unsigned int arity_;

    std::vector<lm::ngram::ChartState> lexical_;
};

} // namespace search

#endif // SEARCH_RULE__
