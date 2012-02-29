#ifndef ALONE_FINAL__
#define ALONE_FINAL__

#include "lm/left.hh"
#include "search/types.hh"

#include <vector>

namespace alone {

class Context;
class Rule;

class Final {
  public:
    Final(const Context &context, const Rule &from, std::vector<const Final *> &children);

    search::Score Total() const { return total_; }

    uint64_t RecombineHash() const { return hash_value(lm_state_); }

    void Recombine(Context &context, Final *with) const;

    const lm::ngram::ChartState &State() const { return lm_state_; }

    const std::vector<const Final *> &Children() const { return children_; }

    const Rule &From() const { return from_; }

  private:
    search::Score total_;
    lm::ngram::ChartState lm_state_;
    const Rule &from_;
    std::vector<const Final *> children_;
};

} // namespace alone

#endif // ALONE_FINAL__
