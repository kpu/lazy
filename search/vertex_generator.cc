#include "search/vertex_generator.hh"

#include "lm/left.hh"
#include "search/context.hh"

#include <stdint.h>

namespace search {

VertexGenerator::VertexGenerator(Context &context, Vertex &gen) : context_(context), edges_(gen.edges_.size()) {
  for (std::size_t i = 0; i < gen.edges_.size(); ++i) {
    if (edges_[i].Init(*gen.edges_[i]))
      generate_.push(&edges_[i]);
  }
  gen.root_.InitRoot();
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
const uint64_t kCompleteAdd = static_cast<uint64_t>(-1);
} // namespace

void VertexGenerator::NewHypothesis(const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {
  std::pair<Existing::iterator, bool> got(existing_.insert(std::pair<uint64_t, Final*>(hash_value(state), NULL)));
  if (!got.second) {
    // Found it already.  
    Final &exists = *got.first->second;
    if (exists.Bound() < partial.score) {
      exists.Reset(partial.score, from.GetRule(), partial.nt[0].End(), partial.nt[1].End());
    }
    --to_pop_;
    return;
  }
  unsigned char left = 0, right = 0;
  Trie *node = &root_;
  while (true) {
    if (left == state.left.length) {
      node = &FindOrInsert(*node, kCompleteAdd - state.left.full, state, left, right);
      for (; right < state.right.length; ++right) {
        node = &FindOrInsert(*node, state.right.words[right], state, left, right + 1);
      }
      node = &FindOrInsert(*node, kCompleteAdd, state, state.left.length, state.right.length);
      break;
    }
    node = &FindOrInsert(*node, state.left.pointers[left], state, left + 1, right);
    left++;
    if (right == state.right.length) {
      node = &FindOrInsert(*node, kCompleteAdd, state, left, right);
      for (; left < state.left.length; ++left) {
        node = &FindOrInsert(*node, state.left.pointers[left], state, left + 1, right);
      }
      node = &FindOrInsert(*node, kCompleteAdd - state.left.full, state, state.left.length, state.right.length);
      break;
    }
    node = &FindOrInsert(*node, state.right.words[right], state, left, right + 1);
    right++;
  }
  got.first->second = CompleteTransition(*node, state, from, partial);
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

Final *VertexGenerator::CompleteTransition(VertexGenerator::Trie &starter, const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {
  VertexNode &node = *starter.under;
  assert(node.State().left.full == state.left.full);
  assert(!node.End());
  Final *final = context_.NewFinal();
  final->Reset(partial.score, from.GetRule(), partial.nt[0].End(), partial.nt[1].End());
  node.SetEnd(final);
  return final;
}

} // namespace search
