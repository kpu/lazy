#ifndef SEARCH_PARTIAL__
#define SEARCH_PARTIAL__

#include "search/types.hh"

#include <boost/unordered_set.hpp>

#include <functional>
#include <queue>
#include <set>

namespace search {

class Partial {
  public:
    typedef std::multi_set<Partial*, GreaterByUpper>::iterator Iter;

    void InitRoot();

    // Upper bound for anything constrained to this node's state.  This should be used by edges.  
    Score Bound() const { return bound_; }

    // Bound excluding the possibility of parents generating more.  This is the sort order of extend_.  
    Score Current() const { return current_; }

    Score GenerateTop() const { return generate_top_; }      

    Word Additional() const { return additional_; }

    ExtendDirection GetDirection() const {
      return direction_;
    }

    void Pressure() {

    }

    // Edges call this when they split.  
    void PushGenerate(const PartialEdge &gen) {
      assert(gen.Score() <= generate_top_);
      generate_.push(gen);
    }

    void PushExtension(Context &context, Word additional, const lm::ngram::ChartState &revised, const PartialEdge &gen) {
      Partial *add = context.ScratchPartial();
      add->additional_ = additional;
      std::pair<boost::unordered_set<Partial*>::iterator, bool> ret(extend_word_.insert(add));
      if (!ret.second) {
        // Already exists.  
        ret.first->PushGenerate(gen);
        return;
      }
      add->Init(this, revised, gen);
      add->parent_iter_ = extend_.insert(add);
      if (add->parent_iter_ == extend_.begin()) {
        SetCurrent(gen.Score());
      }
      context.StealScratchPartial();
    }

  private:
    // Assumes additional_ was set.  Does not set parent_iter_.  
    void Init(Partial *parent, const lm::ngram::ChartState &revised, const PartialEdge &gen) {
      parent_ = parent;
      generate_.clear();
      extend_.clear();
      extend_word_.clear();
      generate_.push(gen);
      generate_top_ = std::max(gen.Score(), parent_->GenerateTop());
      bound_ = generate_top_;
      current_ = gen.Score();
      left_complete_ = parent->left_complete_;
      right_complete_ = parent->right_complete_;
      if (parent->direction_ == kExtendRight) {
        left_ = parent->left_;
        right_ = revised;
      } else {
        left_ = revised;
        right_ = parent_->right_;
      }

      if (!additional_.Terminal()) {
        // This is a completion
        (parent->direction_ == kExtendLeft ? left_complete_ : right_complete_) = true;
      }
      if (left_complete_) {
        direction_ = kExtendRight;
      } else if (right_complete_) {
        direction_ = kExtendLeft;
      } else {
        direction_ = !parent->direction_;
      }
    }

    void PopGenerate(Context &context) {
      Partial *child = generate_.top().Pressure(context, this);
      generate_.pop();

      if (generate_.empty()) {
        generate_top_ = parent_ ? -kScoreInf : parent_->GenerateTop();
      } else {
        generate_top_ = generate_.top().Score();
        if (parent_ && parent->GenerateTop() > generate_top_) generate_top_ = parent->GenerateTop();
      }
      bound_ = std::max(generate_top_, current_);
      // TODO: notify children?  
    }

    void SetCurrent(Score to) {
      if (parent_) {
        parent_->MoveExtension(parent_iter_, to);
      } else {
        current_ = to;
      }
    }

    // Change an extension's score.  Only called by a child.
    void MoveExtension(Iter &iter, Score to) {
      Partial *child = *iter;
      bool was_begin = (iter == extend_.begin());
      iter = extend_.erase(iter);
      child->current_ = to;
      iter = extend_.insert(iter, child);
      if (was_begin) {
        SetCurrent(std::max(generate_top_, (*extend_.begin())->Upper()));
      }
    }

    // The upper bound for this state and anything more specific than it, including the possibility that the parent will assign more edges here.  
    Score bound_;

    // Current best score based on known edges.   
    Score current_;

    Score generate_top_;

    // State completion flags
    bool left_complete_, right_complete_;

    // left or right?
    ExtendDirection direction_;

    struct GreaterByCurrent : public std::binary_function<const Partial*, const Partial*, bool> {
      bool operator()(const Partial *first, const Partial *second) const {
        return first->Current() < second->Current();
      }
    };

    // Extensions can be more on the left or more on the right.   
    std::multi_set<Partial*, GreaterByCurrent> extend_;

    struct HashByAdditional : public std::unary_function<const Partial *, size_t> {
      size_t operator()(const Partial *value) const {
        return hash_value(value->Additional());
      }
    }

    // Lookup extensions by the new word.  
    boost::unordered_set<Partial*> extend_word_;

    std::priority_queue<PartialEdge> generate_;

    Partial *parent_;
    
    // The iterator that gets here from the parent.  
    Iter parent_iter_;

    // May be NULL if this is a state completion transition.  
    Word additional_;

    lm::ngram::ChartState left_, right_;
};

} // namespace search
#endif // SEARCH_PARTIAL__
