#ifndef SEARCH_VERTEX__
#define SEARCH_VERTEX__

#include "lm/left.hh"
#include "search/types.hh"

#include <boost/unordered_set.hpp>

#include <queue>
#include <vector>

#include <stdint.h>

namespace search {

class Edge;
class Final;

class VertexNode {
  public:
    bool Complete() const {
      return extend_.empty();
    }

    const lm::ngram::ChartState &State() const { return state_; }

    Score Bound() const {
      return Complete() ? end_->Bound() : extend_.front().Bound();
    }

    Index Length() const {
      return state_.left.length + state_.right.length;
    }

    const Final *End() const { return end_; }

    const VertexNode &operator[](size_t index) const {
      return extend_[index];
    }

    size_t Size() const {
      return extend_.size();
    }

  private:
    std::vector<VertexNode*> extend_;
    lm::ngram::ChartState state_;
    const Final *end_;
};

class PartialVertex {
  public:
    explicit PartialVertex(const VertexNode &back) : back_(&back), index_(0) {}

    bool Complete() const { return back_->Complete(); }

    const lm::ngram::ChartState &State() const { return back_->State(); }

    Score Bound() const { return Complete() ? back_->End().Bound() : (*back_)[index_].Bound(); }

    Index Length() const { return back_->Length(); }

    bool Split(Context &context, PartialVertex &continuation, PartialVertex &alternate) {
      assert(!Complete());
      continuation.back_ = &(*back_)[index_];
      continuation.index_ = 0;
      if (index_ + 1 < back_.Size()) {
        alternate.back_ = back_;
        alternate.index_ = index_ + 1;
        return true;
      }
      return false;
    }

    const Final *End() const {
      return back_->End();
    }

  private:
    const VertexNode *back_;
    Index index_;
};

class Vertex {
  public:
    Vertex() 
#ifdef DEBUG
      : finished_adding_(false)
#endif
    {}

    void Add(Edge &edge) {
#ifdef DEBUG
      assert(!finished_adding_);
#endif
      edges_.push_back(&edge);
    }

    void FinishedAdding() {
#ifdef DEBUG
      assert(!finished_adding_);
      finished_adding_ = true;
#endif
    }

    PartialVertex &RootPartial() const { return PartialVertex(root_); }

  private:
    friend class VertexGeneratorstd::vector<Edge*>;
    std::vector<Edge*> edges_;

#ifdef DEBUG
    bool finished_adding_;
#endif

    VertexNode root_;
};

} // namespace search
#endif // SEARCH_VERTEX__
