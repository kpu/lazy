#ifndef ALONE_FINAL__
#define ALONE_FINAL__

#include "search/types.hh"

namespace alone {

class Rule;

class Final {
  public:

    search::Score Total() const { return total_; }

    uint64_t RecombineHash() const { return 0; }

    void Recombine(search::Context<Final> &context, Final *with) const {
      context.DeleteFinal(with);
    }

  private:
    const search::Score total_;
    const Rule *const from_;
    std::vector<const Final *> values_;
};

} // namespace alone

#endif // ALONE_FINAL__
