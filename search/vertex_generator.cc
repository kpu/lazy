#include "search/vertex_generator.hh"

#include "lm/left.hh"
#include "search/context.hh"

#include <stdint.h>

namespace search {

VertexGenerator::VertexGenerator(ContextBase &context, Vertex &gen) : context_(context), partial_edge_pool_(sizeof(PartialEdge), context.PopLimit() * 2) {
  gen.root_.InitRoot();
  root_.under = &gen.root_;
  to_pop_ = context.PopLimit();
}

void VertexGenerator::AddEdge(Edge &edge) {
  // Ignore empty edges.  
  for (unsigned int i = 0; i < edge.GetRule().Arity(); ++i) { 
    if (edge.GetVertex(i).RootPartial().Empty()) return;
  }
  generate_.push(edge_pool_.construct(edge, *static_cast<PartialEdge*>(partial_edge_pool_.malloc())));
}

namespace {
const uint64_t kCompleteAdd = static_cast<uint64_t>(-1);
} // namespace

void VertexGenerator::NewHypothesis(const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {
  std::pair<Existing::iterator, bool> got(existing_.insert(std::pair<uint64_t, Final*>(hash_value(state), NULL)));
  if (!got.second) {
    // Found it already.  
    Final &exists = *got.first->second;
    if (exists.Bound() < partial.score) {
      exists.Reset(partial.score, from, partial.nt[0].End(), partial.nt[1].End());
    }
    --to_pop_;
    return;
  }
  unsigned char left = 0, right = 0;
  Trie *node = &root_;
  while (true) {
    if (left == state.left.length) {
      node = &FindOrInsert(*node, kCompleteAdd - state.left.full, state, left, true, right, false);
      for (; right < state.right.length; ++right) {
        node = &FindOrInsert(*node, state.right.words[right], state, left, true, right + 1, false);
      }
      break;
    }
    node = &FindOrInsert(*node, state.left.pointers[left], state, left + 1, false, right, false);
    left++;
    if (right == state.right.length) {
      node = &FindOrInsert(*node, kCompleteAdd - state.left.full, state, left, false, right, true);
      for (; left < state.left.length; ++left) {
        node = &FindOrInsert(*node, state.left.pointers[left], state, left + 1, false, right, true);
      }
      break;
    }
    node = &FindOrInsert(*node, state.right.words[right], state, left, false, right + 1, false);
    right++;
  }

  node = &FindOrInsert(*node, kCompleteAdd - state.left.full, state, state.left.length, true, state.right.length, true);
  got.first->second = CompleteTransition(*node, state, from, partial);
  --to_pop_;
}

VertexGenerator::Trie &VertexGenerator::FindOrInsert(VertexGenerator::Trie &node, uint64_t added, const lm::ngram::ChartState &state, unsigned char left, bool left_full, unsigned char right, bool right_full) {
  VertexGenerator::Trie &next = node.extend[added];
  if (!next.under) {
    next.under = context_.NewVertexNode();
    lm::ngram::ChartState &writing = next.under->MutableState();
    writing = state;
    writing.left.full &= left_full && state.left.full;
    next.under->MutableRightFull() = right_full && state.left.full;
    writing.left.length = left;
    writing.right.length = right;
    node.under->AddExtend(next.under);
  }
  return next;
}

Final *VertexGenerator::CompleteTransition(VertexGenerator::Trie &starter, const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {
  VertexNode &node = *starter.under;
  assert(node.State().left.full == state.left.full);
  assert(!node.End());
  Final *final = context_.NewFinal();
  final->Reset(partial.score, from, partial.nt[0].End(), partial.nt[1].End());
  node.SetEnd(final);
  return final;
}

} // namespace search
