#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/source.hh"
#include "search/types.hh"
#include "search/vertex.hh"

#include <boost/unordered_map.hpp>

#include <ext/pb_ds/priority_queue.hpp>

#include <cmath>
#include <queue>
#include <vector>

#include <stdint.h>

namespace search {

// This class has a 
template <class Rule> class Edge : public Source<typename Rule::Final> {
  public:
    typedef typename Rule::Final Final;
    typedef Vertex<Edge> Child;

  private:
    typedef Source<Final> P;

  public:
    Edge() {}

    Rule &InitRule() { return rule_; }

    void Add(Child &vertex) {
      assert(vertex.Bound() != kScoreInf);
      to_.push_back(&vertex);
    }

    template <class Cont> void FinishedAdding(Cont &context) {
      assert(to_.size() == rule_.Variables());
      context.EnsureIndexPool(rule_.Variables());
      if (to_.empty()) {
        // Special case for purely lexical rules.  
        std::vector<const Final*> empty;
        AddFinal(*context.ApplyRule(context, rule_, empty));
        P::SetBound(-kScoreInf);
      } else {
        // Seed the queue with zero.  
        GenerateEntry entry;
        entry.score = rule_.Bound();
        for (Index i = 0; i < rule_.Variables(); ++i) {
          entry.score += to_[i]->ScoreOrBound(0);
        }
        SetBound(entry.score);
        assert(entry.score != kScoreInf);
        if (entry.score != kScoreInf) {
          entry.indices = context.NewIndices(rule_.Variables());
          memset(entry.indices, 0, sizeof(Index) * rule_.Variables());
          generate_.push(entry);
        }
      }
    }

    template <class Cont> void More(Cont &context, Score beat) {
      // Ease off to beating the holding tank's best score.  
      if (!holding_.empty()) {
        beat = std::max(beat, holding_.top().final->Total());
      }
      GenerateOrLower(context, beat);
      
      SetBound(generate_.empty() ? -kScoreInf : generate_.top().score);
      // Move hypotheses from holding tank to final.   
      while (!holding_.empty()) {
        const HoldingEntry &entry = holding_.top();
        if (entry.final->Total() < P::Bound()) break;
        entry.dedupe->best = entry.final;
        P::AddFinal(*entry.final);
        holding_.pop();
      }
    }

  private:
    template <class Cont> void GenerateOrLower(Cont &context, const Score beat) {
      while (!generate_.empty()) {
        GenerateEntry top(generate_.top());
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
          NewHypothesis(context, top.indices);
          PushNeighbors(context, accumulated, top.indices);
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

    template <class Cont> void NewHypothesis(Cont &context, const Index *indices) {
      std::vector<const Final*> &have_values = context.ClearedTemp();
      for (typename std::vector<Child*>::iterator t = to_.begin(); t != to_.end(); ++t, ++indices) {
        have_values.push_back(&(**t)[*indices]);
      }
      Final *adding = context.ApplyRule(rule_, have_values);

      std::pair<uint64_t, DedupeValue> to_dedupe(adding->RecombineHash(), DedupeValue());
      std::pair<typename Dedupe::iterator, bool> ret(dedupe_.insert(to_dedupe));
      DedupeValue &dedupe_value = ret.first->second;
      if (ret.second) {
        // Unique recombination state.  Push.  
        HoldingEntry holding_entry;
        holding_entry.final = adding;
        holding_entry.dedupe = &dedupe_value;
        // Flag that it's still in the holding tank.  
        dedupe_value.best = NULL;
        dedupe_value.hold = holding_.push(holding_entry);
        return;
      }
      // It's a duplicate.
      if (dedupe_value.best) {
        // Recombine with visible hypothesis.  
        Final *competitor = dedupe_value.best;
        assert(competitor->Total() >= adding->Total());
        competitor->Recombine(context, adding);
        return;
      }
      // Recombine with entry in the holding tank.  
      Final *competitor = dedupe_value.hold->final;
      if (adding->Total() > competitor->Total()) {
        HoldingEntry modified;
        modified.final = adding;
        modified.dedupe = &dedupe_value;
        holding_.modify(dedupe_value.hold, modified);
        adding->Recombine(context, competitor);
        return;
      }
      competitor->Recombine(context, adding);
    }

    // Takes ownership of indices.  
    template <class Cont> void PushNeighbors(Cont &context, Score accumulated, Index *indices) {
      if (to_.empty()) return;
      Index *const end = indices + to_.size();
      const Index *const pre_end = end - 1;
      typename std::vector<Child*>::const_iterator vertex = to_.begin();
      GenerateEntry to_push;
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
            context.DeleteIndices(rule_.Variables(), indices);
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
        to_push.indices = context.NewIndices(rule_.Variables());
        std::copy(indices, end, to_push.indices);
        generate_.push(to_push);
      }
    }

    // Priority queue of potential new hypotheses.  
    struct GenerateEntry {
      Index *indices;
      Score score;

      bool operator<(const GenerateEntry &other) const {
        return score < other.score;
      }
    };
    typedef std::priority_queue<GenerateEntry> Generate;
    Generate generate_;

    // Priority queue of hypotheses that have been generated but not proven to score highest.  
    struct DedupeValue;
    struct HoldingEntry {
      Final *final;
      DedupeValue *dedupe;
      bool operator<(const HoldingEntry &other) const {
        return final->Total() < other.final->Total();
      }
    };
    typedef __gnu_pbds::priority_queue<HoldingEntry> Holding;
    Holding holding_;

    // Deduplication hash table from hypothesis state (hashed to 64-bit as the
    // key) to either the final hypothesis array or the holding tank.  
    struct DedupeValue {
      Final *best;
      typename Holding::point_iterator hold;
    };
    struct IdentityHash : public std::unary_function<uint64_t, uint64_t> {
      uint64_t operator()(uint64_t value) const {
        return value;
      }
    };
    typedef boost::unordered_map<uint64_t, DedupeValue, IdentityHash> Dedupe;
    Dedupe dedupe_;

    // Rule and pointers to rule arguments.  
    Rule rule_;
    std::vector<Child*> to_;
};

} // namespace search
#endif // SEARCH_EDGE__
