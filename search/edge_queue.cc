#include "search/edge_queue.hh"

#include "lm/left.hh"
#include "search/context.hh"

#include <stdint.h>

namespace search {

EdgeQueue::EdgeQueue(unsigned int pop_limit_hint) : partial_edge_pool_(sizeof(PartialEdge), pop_limit_hint * 2) {}

void EdgeQueue::AddEdge(Edge &edge, float total_score) {
  // Ignore empty edges.  
  for (unsigned int i = 0; i < edge.GetRule().Arity(); ++i) {
    PartialVertex root(edge.GetVertex(i).RootPartial());
    if (root.Empty()) return;
    total_score += root.Bound();
  }
  PartialEdge &allocated = *static_cast<PartialEdge*>(partial_edge_pool_.malloc());
  allocated.score = total_score;
  generate_.push(edge_pool_.construct(edge, allocated));
}

} // namespace search
