#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/arity.hh"
#include "search/source.hh"
#include "search/types.hh"
#include "search/vertex.hh"
#include "util/murmur_hash.hh"

#include <boost/array.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <cmath>
#include <queue>
#include <vector>

#include <stdint.h>
#include <string.h>

namespace search {

// This class has a 
template <class Rule> class Edge : public Source<typename Rule::Final> {
  public:
    typedef typename Rule::Final Final;
    typedef Vertex<Edge> Child;

  private:
    typedef Source<Final> P;
    typedef boost::array<search::Index, kMaxArity> Indices;

  public:
    Edge() {
      end_to_ = to_;
    }

    Rule &InitRule() { return rule_; }

    void Add(Child &vertex) {
      assert(vertex.Bound() != kScoreInf);
      assert(end_to_ - to_ < kMaxArity);
      *(end_to_++) = &vertex;
    }

    template <class Cont> void FinishedAdding(Cont &context) {
      assert(to_.size() == rule_.Variables());
      if (to_ == end_to_) {
        // Special case for purely lexical rules.  
        boost::array<const Final*, kMaxArity> empty;
        for (unsigned int i = 0; i < kMaxArity; ++i) {
          empty[i] = NULL;
        }
        AddFinal(*context.ApplyRule(context, rule_, empty));
        P::SetBound(-kScoreInf);
      } else {
        // Seed the queue with zero.  
        GenerateEntry entry;
        entry.score = rule_.Bound();
        for (Index i = 0; i < rule_.Variables(); ++i) {
          entry.score += to_[i]->ScoreOrBound(0);
        }
        P::SetBound(entry.score);
        assert(entry.score != kScoreInf);
        if (entry.score != kScoreInf) {
          memset(entry.indices.c_array(), 0, sizeof(Index) * rule_.Variables());
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
      
      P::SetBound(generate_.empty() ? -kScoreInf : generate_.top().score);
/*      if (dedupe_.size() > 1000) {
        std::cerr << "Giving up on edge " << rule_ << " with " << P::Bound() << '\n';
        P::SetBound(-kScoreInf);
      }*/
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
        if (top.score < beat) return;
        generate_.pop();

        // Recalculate score.  
        Score accumulated = rule_.Bound();
        const Index *indices = top.indices.data();
        Child *pressure = NULL;
        for (Child **t = to_; t != end_to_; ++t, ++indices) {
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
          NewHypothesis(context, top.indices.data());
          PushNeighbors(context, accumulated, top.indices.data());
          return;
        }
        float real_beat;
        if (generate_.empty()) {
          real_beat = beat;
        } else {
          real_beat = std::max(beat, generate_.top().score);
        }
        // Pressure bounds on children.  TODO: better algorithm for this.  
        Index start = pressure->Size();
        top.score = accumulated - pressure->Bound();
        real_beat = pressure->Bound() - (accumulated - real_beat);
        pressure->More(context, real_beat);
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
      boost::array<const Final*, kMaxArity> children;
      const Final **child = children.c_array();
      for (Child **t = to_; t != end_to_; ++t, ++indices, ++child) {
        *child = (&(**t)[*indices]);
      }
      Final *adding = context.ApplyRule(context, rule_, children);

      std::pair<uint64_t, DedupeValue> to_dedupe(adding->RecombineHash(), DedupeValue());
      std::pair<typename Dedupe::iterator, bool> ret(dedupe_.insert(to_dedupe));
      DedupeValue &dedupe_value = ret.first->second;
      if (ret.second) {
        // Unique recombination state.  Push.  
        HoldingEntry holding_entry;
        holding_entry.final = adding;
        holding_entry.dedupe = &dedupe_value;
        dedupe_value.best = NULL;
        dedupe_value.hold = holding_.push(holding_entry);
        return;
      }
      // It's a duplicate.
      if (dedupe_value.best) {
        // Recombine with visible hypothesis.  
//        Final *competitor = dedupe_value.best;
//        assert(competitor->Total() >= adding->Total());
        context.DeleteFinal(adding);
//        competitor->Recombine(context, adding);
        return;
      }
      // Recombine with entry in the holding tank.  
      Final *competitor = (*dedupe_value.hold).final;
      if (adding->Total() > competitor->Total()) {
        HoldingEntry modified;
        modified.final = adding;
        modified.dedupe = &dedupe_value;
        holding_.increase(dedupe_value.hold, modified);
        context.DeleteFinal(competitor);
//        adding->Recombine(context, competitor);
        return;
      }
      context.DeleteFinal(adding);
  //    competitor->Recombine(context, adding);
    }

    template <class Cont> void PushNeighbors(Cont &context, Score accumulated, const Index *indices) {
      if (to_ == end_to_) return;
      Child **vertex = to_;
      GenerateEntry to_push;
      std::copy(indices, indices + rule_.Variables(), to_push.indices.c_array());
      Index *end = to_push.indices.c_array() + rule_.Variables();
      std::size_t index_size = rule_.Variables() * sizeof(Index);
      // Loop is exited by change == pre_end where we either free or use indices.
      // *change is incremented each time then decremented at the end of each loop.  
      for (Index *change = to_push.indices.c_array(); change != end; --*change, ++change, ++vertex) {
        to_push.score = accumulated - (**vertex)[*change].Total();
        ++*change;
        if ((*vertex)->Size() > *change) {
          to_push.score += (**vertex)[*change].Total();
        } else if ((*vertex)->Bound() == -kScoreInf) {
          // Don't bother because there's nothing more from this vertex.  
          continue;
        } else {
          to_push.score += (*vertex)->Bound();
        }
        // TODO: avoid rehash if possible
        std::pair<typename SeenIndices::iterator, bool> seen(seen_indices_.insert(util::MurmurHashNative(to_push.indices.c_array(), index_size, 0)));
        if (seen.second) {
          // Insertion successful.  Add it.  
          generate_.push(to_push);
        } else {
          assert(kMaxArity == 2);
          seen_indices_.erase(seen.first);
        }
      }
    }

    // Priority queue of potential new hypotheses.  
    struct GenerateEntry {
      Indices indices;
      Score score;

      bool operator<(const GenerateEntry &other) const {
        return score < other.score;
      }
    };
    typedef std::priority_queue<GenerateEntry> Generate;
    Generate generate_;

    struct IdentityHash : public std::unary_function<uint64_t, uint64_t> {
      uint64_t operator()(uint64_t value) const {
        return value;
      }
    };

    typedef boost::unordered_set<uint64_t, IdentityHash> SeenIndices;
    SeenIndices seen_indices_;

    // Priority queue of hypotheses that have been generated but not proven to score highest.  
    struct DedupeValue;
    struct HoldingEntry {
      Final *final;
      DedupeValue *dedupe;
      bool operator<(const HoldingEntry &other) const {
        return final->Total() < other.final->Total();
      }
    };
    typedef boost::heap::fibonacci_heap<HoldingEntry> Holding;
    Holding holding_;

    // Deduplication hash table from hypothesis state (hashed to 64-bit as the
    // key) to either the final hypothesis array or the holding tank.  
    struct DedupeValue {
      Final *best;
      typename Holding::handle_type hold;
    };
    typedef boost::unordered_map<uint64_t, DedupeValue, IdentityHash> Dedupe;
    Dedupe dedupe_;

    // Rule and pointers to rule arguments.  
    Rule rule_;

    Child *to_[kMaxArity];
    Child **end_to_;
};

} // namespace search
#endif // SEARCH_EDGE__
