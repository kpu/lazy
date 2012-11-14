#include "alone/assemble.hh"

#include "alone/edge.hh"
#include "alone/features.hh"
#include "search/applied.hh"

#include <iostream>

namespace alone {

std::ostream &JustText(std::ostream &o, const search::Applied final) {
  const Edge::WordVec &words = static_cast<const Edge*>(final.GetNote().vp)->Words();
  if (words.empty()) return o;
  const search::Applied *child = final.Children();
  Edge::WordVec::const_iterator i(words.begin());
  // Initial case: check for <s>.
  if (*i) {
    if ((*i)->first != "<s>")
      o << (*i)->first << ' ';
    if (++i == words.end()) return o;
  }
  // Middle case.  
  for (; i != words.end() - 1; ++i) {
    if (*i) {
      o << (*i)->first << ' ';
    } else {
      JustText(o, *child) << ' ';
      ++child;
    }
  }

  // Final case: check for </s>.
  if (*i) {
    if ((*i)->first != "</s>")
      o << (*i)->first;
  } else {
    JustText(o, *child);
  }

  return o;
}

namespace {
void RecurseFeatures(const search::Applied final, std::vector<lm::WordIndex> &words, feature::Adder &adder) {
  const Edge &edge = *static_cast<const Edge*>(final.GetNote().vp);
  adder.Add(edge.Features());
  const Edge::WordVec &vec = edge.Words();
  const search::Applied *child = final.Children();
  for (Edge::WordVec::const_iterator i(vec.begin()); i != vec.end(); ++i) {
    if (*i) {
      words.push_back((*i)->second);
    } else {
      RecurseFeatures(*child++, words, adder);
    }
  }
}
} // namespace

void ComputeForFeatures(const search::Applied final, std::vector<lm::WordIndex> &words, feature::Vector &vec) {
  assert(final.Valid());
  feature::Adder adder;
  RecurseFeatures(final, words, adder);
  adder.Finish(vec);
}

namespace {

void MakeIndent(std::ostream &o, const char *indent_str, unsigned int level) {
  for (unsigned int i = 0; i < level; ++i)
    o << indent_str;
}

void DetailedAppliedInternal(std::ostream &o, const search::Applied final, const char *indent_str, unsigned int indent) {
  o << "(\n";
  MakeIndent(o, indent_str, indent);
  const Edge::WordVec &words = static_cast<const Edge*>(final.GetNote().vp)->Words();
  const search::Applied *child = final.Children();
  for (Edge::WordVec::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (*i) {
      o << (*i)->first;
      if (i == words.end() - 1) {
        o << '\n';
        MakeIndent(o, indent_str, indent);
      } else {
        o << ' ';
      }
    } else {
      // One extra indent from the line we're currently on.  
      o << indent_str;
      DetailedAppliedInternal(o, *child, indent_str, indent + 1);
      for (unsigned int i = 0; i < indent; ++i) o << indent_str;
      ++child;
    }
  }
  o << ")=" << final.GetScore() << '\n';
}
} // namespace

void DetailedApplied(std::ostream &o, const search::Applied final, const char *indent_str) {
  DetailedAppliedInternal(o, final, indent_str, 0);
}

} // namespace alone
