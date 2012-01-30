#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/source.hh"
#include "search/types.hh"

#include <cmath>
#include <queue>
#include <vector>

namespace search {

template <class Rule> class Edge : public Source<typename Rule::Final> {
  public:
    typedef typename Rule::Final Final;
    typedef Vertex<Edge> Vertex;

  private:
    typedef Source<Final> P;

    struct QueueEntry {
      Index *indices;
      Score score;

      bool operator<(const QueueEntry &other) const {
        return score > other.score;
      }
    };

  public:
    explicit Edge(const Rule &rule) 
      : rule_(rule), index_pool_(sizeof(Index) * rule_.Variables()) {}

    void AddVertex(Vertex &vertex) {
      assert(vertex.Bound() != kScoreInf);
      to_.push_back(&vertex);
    }

    void FinishedAdding(Context &context) {
      assert(to_.size() == rule_.Variables());
      if (to_.empty()) {
        // Special case for purely lexical rules.  
        std::vector<const Final*> empty;
        Final *final = context.NewFinal();
        rule_.Apply(empty, final);
        AddFinal(*final);
        SetBound(-kScoreInf);
      } else {
        // Seed the queue with zero.  
        QueueEntry entry;
        entry.score = rule_.Score();
        for (Index i = 0; i < rule_.Variables(); ++i) {
          Score got = to_[i]->ScoreOrBound(0);
          entry.score += to_[i]->ScoreOrBound(0);
        }
        SetBound(entry.score);
        assert(entry.score != kScoreInf);
        if (entry.score != kScoreInf) {
          entry.indices = index_pool_.malloc();
          if (!entry.indices) throw std::bad_alloc();
          memset(entry.indices, 0, sizeof(Index) * rule_.Variables());
          generate_.push(entry);
        }
      }
    }

    void More(Context &context, Score beat) {
      // Ease off to beating the holding tank's best score.  
      if (!holding_.empty()) {
        beat = std::max(beat, holding_.top().Score());
      }
      GenerateOrLower(context, beat);
      
      SetBound(generate_.empty() ? -kScoreInf : generate_.top().score);
      // Move hypotheses from holding tank to final.   
      while (!holding_.empty()) {
        const Final &top = holding_.top();
        if (top.Score() < Bound()) break;
        final_.push_back(top);
        holding_.pop();
      }
    }

  private:
    void GenerateOrLower(Context &context, const Score beat) {
      std::vector<Vertex*> open_vertices;
      while (!generate_.empty()) {
        QueueEntry top(generate_.top());
        assert(top.score != -kScoreInf);
        assert(top.score != kScoreInf);
        if (top.score < beat) return;
        generate_.pop();

        // Recalculate score.  
        Score accumulated = rule_.Score();
        Index *indices = top.indices;
        for (typename std::vector<Vertex*>::iterator t = to_.begin(); t != to_.end(); ++t, ++indices) {
          Vertex &vertex = **t;
          if (vertex.Size() > *indices) {
            accumulated += vertex[*indices].Score();
          } else {
            accumulated += vertex.Bound();
            open_vertices.push_back(&vertex);
          }
        }
        // If the score went down, put it back.  
        if (accumulated < top.score) {
          if (accumulated == -kScoreInf) continue;
          top.score = accumulated;
          generate_.push(top);
          continue;
        }
        if (open_vertices.empty()) {
          // Every variable has a value.  
          std::vector<const Final*> &have_values = context.ClearTemp();
          indices = top.indices;
          for (typename std::vector<Vertex*>::iterator t = to_.begin(); t != to_.end(); ++t, ++indices) {
            have_values.push_back(&(*t)[*indices]);
          }
          Final *final = context.NewFinal();
          rule_.Apply(have_values, *final);
          holding_.push(final);
          PushNeighbors(accumulated, top.indices);
          return;
        }
        // Pressure bounds on children.  
        Score sub_beat = beat;
        if (!generate_.empty()) {
          sub_beat = std::max(sub_beat, generate_.top().score);
        }
        top.score = PressureChildren(context, open_vertices, accumulated, sub_beat);
        if (top.score != -kScoreInf) generate_.push(top);
      }
    }

    Score PressureChildren(Context &context, std::vector<Vertex*> &open_vertices, Score accumulated, const Score beat) {
      assert(!open_vertices.empty());
      // TODO: investigate algorithms.  
      for (std::vector<Vertex*>::iterator i = open_vertices.begin(); ; ) {
        Vertex &vertex = **i;
        Index pre_size = vertex.Size();
        accumulated -= vertex.Bound();
        vertex.More(context, vertex.Bound());
        if (vertex.Size() > pre_size) {
          accumulated += vertex[pre_size].Score();
          i = open_vertices.erase(i);
          if (open_vertices.empty()) return accumulated;
        } else {
          accumulated += vertex.Bound();
          ++i;
        }
        if (accumulated < beat) return accumulated;
        if (i == open_vertices.end()) i = open_vertices.begin();
      }
    }

    // Takes ownership of indices.  
    void PushNeighbors(Score accumulated, Index *indices) {
      if (to_.empty()) return;
      const Index *const end = indices + to_.size();
      const Index *const pre_end = end - 1;
      std::vector<Vertex*>::const_iterator vertex = to_.begin();
      QueueEntry to_push;
      // Loop is exited by change == pre_end where we either free or use indices.
      // *change is incremented each time then decremented at the end of each loop.  
      for (const Index *change = indices; ; --*change, ++change, ++vertex) {
        to_push.score = accumulated - (*vertex)[*change].Score();
        ++*change;
        if (vertex->Size() > *change) {
          to_push.score += (*vertex)[*change].Score();
        } else if (vertex->Bound() == -kScoreInf) {
          // Don't bother because there's nothing more from this vertex.  
          if (change == pre_end) {
            index_pool_.free(indices);
            break;
          }
          continue;
        } else {
          to_push.score += vertex->Bound();
        }
        if (change == pre_end) {
          to_push.indices = indices;
          generate_.push(to_push);
          break;
        }
        to_push.indices = index_pool_.malloc();
        if (!to_push.indices) throw std::bad_alloc();
        std::copy(indices, end, to_push.indices);
        generate_.push(to_push);
      }
    }

    struct FinalGreater : public std::binary_function<const Final *, const Final *, bool> {
      bool operator()(const Final *first, const Final *second) const {
        return final->Score() > second->Score();
      }
    };

    std::priority_queue<const Final *, std::vector<const Final*>, FinalGreater> holding_;

    // Rule and pointers to rule arguments.  
    Rule rule_;
    std::vector<Vertex*> to_;

    boost::pool<> index_pool_;

    std::priority_queue<QueueEntry> generate_;
};

} // namespace search
#endif // SEARCH_EDGE__
