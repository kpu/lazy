#include "alone/assemble.hh"

#include "search/final.hh"
#include "search/rule.hh"

namespace alone {

std::ostream &operator<<(std::ostream &o, const search::Final &final) {
  const search::Rule::ItemsRet &words = final.From().Items();
  const search::Final *const *child = final.Children().data();
  for (search::Rule::ItemsRet::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (i->Terminal()) {
      o << i->String() << ' ';
    } else {
      o << **child;
      ++child;
    }
  }
  return o;
}

void DetailedFinal(std::ostream &o, const search::Final &final) {
  o << '(';
  const search::Rule::ItemsRet &words = final.From().Items();
  const search::Final *const *child = final.Children().data();
  for (search::Rule::ItemsRet::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (i->Terminal()) {
      o << i->String() << ' ';
    } else {
      DetailedFinal(o, **child);
      ++child;
    }
  }
  o << ")=" << final.Bound() << ' ';
}

} // namespace alone
