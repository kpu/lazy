#include "alone/lattice_output.hh"

#include "alone/edge.hh"
#include "search/vertex.hh"

#include <cmath>
#include <iostream>

namespace alone {

search::NBestComplete LatticeOutput::Complete(std::vector<search::PartialEdge> &partials) {
  using namespace search;
  if (partials.empty())
    return NBestComplete(NULL, lm::ngram::ChartState(), -INFINITY);
  std::cout << "Vertex " << (counter_++) << ' ' << partials.size() << '\n';
  Score max_score = -INFINITY;
  for (std::vector<PartialEdge>::const_iterator i = partials.begin(); i != partials.end(); ++i) {
    max_score = std::max(max_score, i->GetScore());
    // Convert total score (best path to here) to relative score.
    Score relative = i->GetScore();
    std::cout << (static_cast<const Edge*>(i->GetNote().vp) - edge_base_) << ' ' << i->GetArity();
    for (Arity a = 0; a < i->GetArity(); ++a) {
      std::cout << ' ' << reinterpret_cast<intptr_t>(i->NT()[a].End());
      relative -= i->NT()[a].Bound();
    }
    std::cout << ' ' << relative;
  }
  return NBestComplete(reinterpret_cast<void*>(counter_), partials.front().CompletedState(), max_score);
}

} // namespace alone
