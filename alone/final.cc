#include "alone/final.hh"

#include "alone/context.hh"
#include "alone/rule.hh"

namespace alone {

Final::Final(const Context &context, const Rule &from, std::vector<const Final *> &children) : from_(from) {
  lm::ngram::RuleScore<lm::ngram::Model> scorer(context.LanguageModel(), lm_state_);
  // Steal the vector of children non-terminals.    
  std::swap(children_, children);
  std::vector<const Final*>::const_iterator child(children_.begin());
  for (std::vector<Word>::const_iterator i = from.Items().begin(); i != from.Items().end(); ++i) {
    if (i->Terminal()) {
      scorer.Terminal(i->Index());
    } else {
      scorer.NonTerminal((*child)->State(), (*child)->Total());
    }
  }
  total_ = from.Additive() + scorer.Finish();
}

void Final::Recombine(Context &context, Final *with) const {
  context.DeleteFinal(with);
}

} // namespace alone
