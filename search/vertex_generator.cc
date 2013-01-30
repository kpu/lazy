#include "search/vertex_generator.hh"

#include "lm/left.hh"
#include "search/context.hh"
#include "search/edge.hh"

#include <boost/unordered_map.hpp>
#include <boost/version.hpp>

#include <stdint.h>

namespace search {

#if BOOST_VERSION > 104200
namespace {

const uint64_t kCompleteAdd = static_cast<uint64_t>(-1);

void FindOrInsert(ContextBase &context, Trie *&node, uint64_t added, const lm::ngram::ChartState &state, unsigned char left, bool left_full, unsigned char right, bool right_full) {
  Trie &next = node->extend[added];
  if (!next.under) {
    next.under = context.NewVertexNode();
    lm::ngram::ChartState &writing = next.under->MutableState();
    // Fill in right state.
    std::copy(state.right.words, state.right.words + right, writing.right.words);
    std::copy(state.right.backoff, state.right.backoff + right, writing.right.backoff);
    writing.right.length = right;
    next.under->MutableRightFull() = right_full && state.left.full;

    // Fill in left state.
    std::copy(state.left.pointers, state.left.pointers + left, writing.left.pointers);
    writing.left.full = left_full && state.left.full;
    writing.left.length = left;

    node->under->AddExtend(next.under);
  }
  node = &next;
}

} // namespace

TreeMaker::TreeMaker(ContextBase &context, Vertex &gen) : context_(context) {
  gen.root_.InitRoot();
  gen.left_.InitRoot();
  gen.right_.InitRoot();
  alternate_.under = &gen.root_;
  left_.under = &gen.left_;
  right_.under = &gen.right_;
}

void TreeMaker::AddHypothesis(const NBestComplete &end) {
  const lm::ngram::ChartState &state = *end.state;
  
  // Alternating
  unsigned char left = 0, right = 0;
  Trie *node = &alternate_;
  while (true) {
    if (left == state.left.length) {
      FindOrInsert(context_, node, kCompleteAdd - state.left.full, state, left, true, right, false);
      for (; right < state.right.length; ++right) {
        FindOrInsert(context_, node, state.right.words[right], state, left, true, right + 1, false);
      }
      break;
    }
    FindOrInsert(context_, node, state.left.pointers[left], state, left + 1, false, right, false);
    left++;
    if (right == state.right.length) {
      FindOrInsert(context_, node, kCompleteAdd - state.left.full, state, left, false, right, true);
      for (; left < state.left.length; ++left) {
        FindOrInsert(context_, node, state.left.pointers[left], state, left + 1, false, right, true);
      }
      break;
    }
    FindOrInsert(context_, node, state.right.words[right], state, left, false, right + 1, false);
    right++;
  }

  FindOrInsert(context_, node, kCompleteAdd - state.left.full, state, state.left.length, true, state.right.length, true);
  node->under->SetEnd(end.history, end.score);

  // Left words only.
  node = &left_;
  for (left = 0; left < state.left.length; ++left) {
    FindOrInsert(context_, node, state.left.pointers[left], state, left, false, 0, false);
  }
  if (state.left.full) {
    // One node for completion.
    FindOrInsert(context_, node, kCompleteAdd - 1, state, state.left.length, true, 0, false);
    // One node for all of right state.  TODO don't use hash table for this.
    FindOrInsert(context_, node, hash_value(state.right), state, state.left.length, true, state.right.length, true);
  } else {
    // Jump straight to the conclusion since left determines right anyway.
    FindOrInsert(context_, node, kCompleteAdd, state, state.left.length, true, state.right.length, true);
  }
  node->under->SetEnd(end.history, end.score);

  // Right words only.
  node = &right_;
  for (right = 0; right < state.right.length; ++right) {
    FindOrInsert(context_, node, state.right.words[right], state, 0, false, right, false);
  }
  if (state.left.full) {
    // One node for completion.
    FindOrInsert(context_, node, kCompleteAdd - 1, state, 0, false, state.right.length, true);
    // One node for all of right state.  TODO don't use hash table for this.
    FindOrInsert(context_, node, hash_value(state.left), state, state.left.length, true, state.right.length, true);
  } else {
    // Jump straight to the conclusion.
    FindOrInsert(context_, node, kCompleteAdd, state, state.left.length, true, state.right.length, true);
  }
  node->under->SetEnd(end.history, end.score);
}

void TreeMaker::Finish() {
  alternate_.under->SortAndSet(context_);
  left_.under->SortAndSet(context_);
  right_.under->SortAndSet(context_);
}

#endif // BOOST_VERSION

} // namespace search
