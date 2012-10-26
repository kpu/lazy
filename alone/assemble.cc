#include "alone/assemble.hh"

#include "alone/edge_words.hh"
#include "search/final.hh"

#include <iostream>

namespace alone {

std::ostream &operator<<(std::ostream &o, const search::Final final) {
  const std::vector<const std::string*> &words = static_cast<const EdgeWords *>(final.GetNote().vp)->Words();
  if (words.empty()) return o;
  const search::Final *child = final.Children();
  std::vector<const std::string*>::const_iterator i(words.begin());
  for (; i != words.end() - 1; ++i) {
    if (*i) {
      o << **i << ' ';
    } else {
      o << *child << ' ';
      ++child;
    }
  }

  if (*i) {
    if (**i != "</s>") {
      o << **i;
    }
  } else {
    o << *child;
  }

  return o;
}

namespace {

void MakeIndent(std::ostream &o, const char *indent_str, unsigned int level) {
  for (unsigned int i = 0; i < level; ++i)
    o << indent_str;
}

void DetailedFinalInternal(std::ostream &o, const search::Final final, const char *indent_str, unsigned int indent) {
  o << "(\n";
  MakeIndent(o, indent_str, indent);
  const std::vector<const std::string*> &words = static_cast<const EdgeWords *>(final.GetNote().vp)->Words();
  const search::Final *child = final.Children();
  for (std::vector<const std::string*>::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (*i) {
      o << **i;
      if (i == words.end() - 1) {
        o << '\n';
        MakeIndent(o, indent_str, indent);
      } else {
        o << ' ';
      }
    } else {
      // One extra indent from the line we're currently on.  
      o << indent_str;
      DetailedFinalInternal(o, *child, indent_str, indent + 1);
      for (unsigned int i = 0; i < indent; ++i) o << indent_str;
      ++child;
    }
  }
  o << ")=" << final.GetScore() << '\n';
}
} // namespace

void DetailedFinal(std::ostream &o, const search::Final final, const char *indent_str) {
  DetailedFinalInternal(o, final, indent_str, 0);
}

} // namespace alone
