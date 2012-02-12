#ifndef ALONE_CONTEXT__
#define ALONE_CONTEXT__

#include "alone/final.hh"
#include "search/context.hh"

namespace alone {

class Context : public search::Context<Final> {
  public:
    Context(const char *file) : lm_(file) {}

    const lm::ngram::Model &LanguageModel() const { return lm_; }

  private:
    lm::ngram::Model lm_;
};

} // namespace alone
#endif // ALONE_CONTEXT__
