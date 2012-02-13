#include "alone/context.hh"
#include "alone/final.hh"
#include "alone/rule.hh"

namespace alone {

Final::Final(const Context &context, const Rule &from, std::vector<const Final *> &children) : from_(from) {
  std::swap(children_, children);
  total_ = from.Apply(context, children_, lm_state_);
}

void Final::Recombine(Context &context, Final *with) const {
  context.DeleteFinal(with);
}

} // namespace alone
