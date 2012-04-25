#include "search/vertex_generator.hh"

#include "lm/left.hh"
#include "search/context.hh"

#include <stdint.h>

namespace search {

VertexGenerator::VertexGenerator(Context &context, Vertex &gen) : context_(context), edges_(gen.edges_.size()) {
  for (std::size_t i = 0; i < gen.edges_.size(); ++i) {
    edges_[i].Init(*gen.edges_[i]);
    generate_.push(&edges_[i]);
  }
  root_.under = &gen.root_;
  to_pop_ = context.PopLimit();
  while (to_pop_ > 0 && !generate_.empty()) {
    EdgeGenerator *top = generate_.top();
    generate_.pop();
    if (top->Pop(context, *this)) {
      generate_.push(top);
    }
  }
  gen.root_.SortAndSet();
}

namespace {
const uint64_t kCompletionAdd = static_cast<uint64_t>(-1);
} // namespace


void VertexGenerator::NewHypothesis(const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {
  unsigned char left = 0, right = 0;
  Trie *node = &root_;
  while (true) {
    if (left == state.left.length) break;
    node = &FindOrInsert(*node, state.left.pointers[left], state, left + 1, right);
    left++;
    if (right == state.right.length) break;
    node = &FindOrInsert(*node, state.right.words[right], state, left, right + 1);
    right++;
  }
  node = &FindOrInsert(*node, kCompletionAdd, state, left, right);
  if (left == state.left.length) {
    for (; right < state.right.length; ++right) {
      node = &FindOrInsert(*node, state.right.words[right], state, left, right + 1);
    }
  } else {
    for (; left < state.left.length; ++left) {
      node = &FindOrInsert(*node, state.left.pointers[left], state, left + 1, right);
    }
  }
  CompleteTransition(*node, state, from, partial);
  --to_pop_;
}

VertexGenerator::Trie &VertexGenerator::FindOrInsert(VertexGenerator::Trie &node, uint64_t added, const lm::ngram::ChartState &state, unsigned char left, unsigned char right) {
  VertexGenerator::Trie &next = node.extend[added];
  if (!next.under) {
    next.under = context_.NewVertexNode();
    lm::ngram::ChartState &writing = next.under->MutableState();
    writing = state;
    writing.left.full &= (left == state.left.length);
    writing.left.length = left;
    writing.right.length = right;
    node.under->AddExtend(next.under);
  }
  return next;
}

void VertexGenerator::CompleteTransition(VertexGenerator::Trie &starter, const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {
  VertexNode &node = *FindOrInsert(starter, kCompletionAdd, state, state.left.length, state.right.length).under;
  assert(node.State().left.full == state.left.full);
  if (!node.End()) {
    Final *final = context_.NewFinal();
    final->Reset(partial.score, from.GetRule(), partial.nt[0].End(), partial.nt[1].End());
    node.SetEnd(final);
    return;
  }
  if (node.End()->Bound() >= partial.score) return;
  node.MutableEnd().Reset(partial.score, from.GetRule(), partial.nt[0].End(), partial.nt[1].End());
}

} // namespace search
