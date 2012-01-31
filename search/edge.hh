#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/context.hh"
#include "search/source.hh"
#include "search/types.hh"
#include "search/vertex.hh"

#include <cmath>
#include <queue>
#include <vector>

namespace search {

template <class Rule> class Edge : public Source<typename Rule::Final> {
  public:
    typedef typename Rule::Final Final;
    typedef Vertex<Edge> Child;

  private:
    typedef Source<Final> P;

    struct QueueEntry {
      Index *indices;
      Score score;

      bool operator<(const QueueEntry &other) const {
        return score < other.score;
      }
    };

  public:
    explicit Edge(const Rule &rule) 
      : rule_(rule), index_pool_(sizeof(Index) * rule_.Variables()) {}

    void Add(Child &vertex) {
      assert(vertex.Bound() != kScoreInf);
      to_.push_back(&vertex);
    }

    void FinishedAdding(Context<Final> &context) {
      assert(to_.size() == rule_.Variables());
      if (to_.empty()) {
        // Special case for purely lexical rules.  
        std::vector<const Final*> empty;
        Final *final = context.NewFinal();
        rule_.Apply(empty, *final);
        AddFinal(*final);
        P::SetBound(-kScoreInf);
      } else {
        // Seed the queue with zero.  
        QueueEntry entry;
        entry.score = rule_.Bound();
        for (Index i = 0; i < rule_.Variables(); ++i) {
          entry.score += to_[i]->ScoreOrBound(0);
        }
        SetBound(entry.score);
        assert(entry.score != kScoreInf);
        if (entry.score != kScoreInf) {
          entry.indices = static_cast<Index*>(index_pool_.malloc());
          if (!entry.indices) throw std::bad_alloc();
          memset(entry.indices, 0, sizeof(Index) * rule_.Variables());
          generate_.push(entry);
        }
      }
    }

    void More(Context<Final> &context, Score beat) {
      // Ease off to beating the holding tank's best score.  
      if (!holding_.empty()) {
        beat = std::max(beat, holding_.top()->Total());
      }
      GenerateOrLower(context, beat);
      
      SetBound(generate_.empty() ? -kScoreInf : generate_.top().score);
      // Move hypotheses from holding tank to final.   
      while (!holding_.empty()) {
        const Final &top = *holding_.top();
        if (top.Total() < P::Bound()) break;
        P::AddFinal(top);
        holding_.pop();
      }
    }

  private:
    void GenerateOrLower(Context<Final> &context, const Score beat) {
      while (!generate_.empty()) {
        QueueEntry top(generate_.top());
        assert(top.score != -kScoreInf);
        assert(top.score != kScoreInf);
        if (top.score < beat) return;
        generate_.pop();

        // Recalculate score.  
        Score accumulated = rule_.Bound();
        Index *indices = top.indices;
        Child *pressure = NULL;
        for (typename std::vector<Child*>::iterator t = to_.begin(); t != to_.end(); ++t, ++indices) {
          Child &vertex = **t;
          if (vertex.Size() > *indices) {
            accumulated += vertex[*indices].Total();
          } else {
            accumulated += vertex.Bound();
            pressure = &vertex;
          }
        }
        // If the score went down, put it back.  
        if (accumulated < top.score) {
          if (accumulated == -kScoreInf) continue;
          top.score = accumulated;
          generate_.push(top);
          continue;
        }
        if (!pressure) {
          // Every variable has a value.  
          std::vector<const Final*> &have_values = context.ClearedTemp();
          indices = top.indices;
          for (typename std::vector<Child*>::iterator t = to_.begin(); t != to_.end(); ++t, ++indices) {
            have_values.push_back(&(**t)[*indices]);
          }
          Final *final = context.NewFinal();
          rule_.Apply(have_values, *final);
          holding_.push(final);
          PushNeighbors(accumulated, top.indices);
          return;
        }
        // Pressure bounds on children.  TODO: better algorithm for this.  
        Index start = pressure->Size();
        top.score = accumulated - pressure->Bound();
        // This might actually impact siblings, so recomputing is not unreasonable.  
        pressure->More(context, pressure->Bound());
        if (pressure->Size() > start) {
          top.score += (*pressure)[start].Total();
          generate_.push(top);
        } else if (pressure->Bound() != -kScoreInf) {
          top.score += pressure->Bound();
          generate_.push(top);
        }
      }
    }

    // Takes ownership of indices.  
    void PushNeighbors(Score accumulated, Index *indices) {
      if (to_.empty()) return;
      Index *const end = indices + to_.size();
      const Index *const pre_end = end - 1;
      typename std::vector<Child*>::const_iterator vertex = to_.begin();
      QueueEntry to_push;
      // Loop is exited by change == pre_end where we either free or use indices.
      // *change is incremented each time then decremented at the end of each loop.  
      for (Index *change = indices; ; --*change, ++change, ++vertex) {
        to_push.score = accumulated - (**vertex)[*change].Total();
        ++*change;
        if ((*vertex)->Size() > *change) {
          to_push.score += (**vertex)[*change].Total();
        } else if ((*vertex)->Bound() == -kScoreInf) {
          // Don't bother because there's nothing more from this vertex.  
          if (change == pre_end) {
            index_pool_.free(indices);
            break;
          }
          continue;
        } else {
          to_push.score += (*vertex)->Bound();
        }
        if (change == pre_end) {
          to_push.indices = indices;
          generate_.push(to_push);
          break;
        }
        to_push.indices = static_cast<Index*>(index_pool_.malloc());
        if (!to_push.indices) throw std::bad_alloc();
        std::copy(indices, end, to_push.indices);
        generate_.push(to_push);
      }
    }

    struct FinalLess : public std::binary_function<const Final *, const Final *, bool> {
      bool operator()(const Final *first, const Final *second) const {
        return first->Total() < second->Total();
      }
    };

    std::priority_queue<const Final *, std::vector<const Final*>, FinalLess> holding_;

    // Rule and pointers to rule arguments.  
    const Rule rule_;
    std::vector<Child*> to_;

    boost::pool<> index_pool_;

    std::priority_queue<QueueEntry> generate_;
};

} // namespace search
#endif // SEARCH_EDGE__
