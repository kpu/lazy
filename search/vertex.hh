#ifndef SEARCH_VERTEX__
#define SEARCH_VERTEX__

#include "lm/left.hh"
#include "search/types.hh"

#include <boost/unordered_set.hpp>

#include <queue>
#include <vector>

#include <math.h>
#include <stdint.h>

namespace search {

class ContextBase;

class VertexNode {
  public:
    VertexNode() : end_() {}

    void InitRoot() {
      extend_.clear();
      state_.left.full = false;
      state_.left.length = 0;
      state_.right.length = 0;
      right_full_ = false;
      niceness_ = 0;
      end_ = History();
    }

    lm::ngram::ChartState &MutableState() { return state_; }
    bool &MutableRightFull() { return right_full_; }

    void AddExtend(VertexNode *next) {
      extend_.push_back(next);
    }

    void SetEnd(History end, Score score) {
      assert(!end_);
      end_ = end;
      bound_ = score;
    }
    
    void SortAndSet(ContextBase &context);

    // Should only happen to a root node when the entire vertex is empty.   
    bool Empty() const {
      return !end_ && extend_.empty();
    }

    bool Complete() const {
      return end_;
    }

    const lm::ngram::ChartState &State() const { return state_; }
    bool RightFull() const { return right_full_; }

    void SetNiceness(unsigned char to) { niceness_ = to; }

    // Priority relative to other non-terminals.  0 is highest.
    unsigned char Niceness() const {
      return niceness_;
    }

    Score Bound() const {
      return bound_;
    }

    // Will be invalid unless this is a leaf.   
    const History End() const { return end_; }

    const VertexNode &operator[](size_t index) const {
      return *extend_[index];
    }

    size_t Size() const {
      return extend_.size();
    }

  private:
    void RecursiveSortAndSet(ContextBase &context, VertexNode *&parent);

    std::vector<VertexNode*> extend_;

    lm::ngram::ChartState state_;
    bool right_full_;

    unsigned char niceness_;

    Score bound_;
    History end_;
};

class PartialVertex {
  public:
    PartialVertex() {}

    explicit PartialVertex(const VertexNode &back) : back_(&back), index_(0) {}

    bool Empty() const { return back_->Empty(); }

    bool Complete() const { return back_->Complete(); }

    const lm::ngram::ChartState &State() const { return back_->State(); }
    bool RightFull() const { return back_->RightFull(); }

    Score Bound() const { return Complete() ? back_->Bound() : (*back_)[index_].Bound(); }

    unsigned char Niceness() const { return back_->Niceness(); }

    bool HasAlternative() const {
      return index_ + 1 < back_->Size();
    }

    // Split into continuation and alternative, rendering this the continuation.
    bool Split(PartialVertex &alternative) {
      assert(!Complete());
      bool ret;
      if (index_ + 1 < back_->Size()) {
        alternative.index_ = index_ + 1;
        alternative.back_ = back_;
        ret = true;
      } else {
        ret = false;
      }
      back_ = &((*back_)[index_]);
      index_ = 0;
      return ret;
    }

    const History End() const {
      return back_->End();
    }

  private:
    const VertexNode *back_;
    unsigned int index_;
};

template <class Output> class VertexGenerator;

class Vertex {
  public:
    Vertex() {}

    PartialVertex RootFirst() const { return PartialVertex(right_); }
    PartialVertex RootAlternate() const { return PartialVertex(root_); }
    PartialVertex RootLast() const { return PartialVertex(left_); }

    bool Empty() const {
      return root_.Empty();
    }

    Score Bound() const {
      return root_.Bound();
    }

    const History BestChild() const {
      // left_ and right_ are not set at the root.
      PartialVertex top(RootAlternate());
      if (top.Empty()) {
        return History();
      } else {
        PartialVertex continuation;
        while (!top.Complete()) {
          top.Split(continuation);
        }
        return top.End();
      }
    }

  private:
    friend class TreeMaker;

    template <class Output> friend class RootVertexGenerator;
    VertexNode root_;

    // These will not be set for the root vertex.
    // Branches only on left state.
    VertexNode left_;
    // Branches only on right state.
    VertexNode right_;
};

} // namespace search
#endif // SEARCH_VERTEX__
