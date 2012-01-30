#ifndef SEARCH_VERTEX__
#define SEARCH_VERTEX__

#include "search/source.hh"
#include "search/types.hh"

#include <queue>
#include <vector>

namespace search {

// Option is typically an instantiation of Edge.
template <class Option> class Vertex : public Source<typename Option::Final> {
  public:
    typedef typename Option::Final Final;

  private:
    typedef Source<Final> P;

    struct QueueEntry {
      Option *edge;
      Index index;
      // Cached score.  
      Score score;
      // Priority queue's order is annoying.  
      bool operator<(const QueueEntry &other) const {
        return score > other.score;
      }
    };

  public:
    Vertex() {}

    void AddEdge(Option *edge) {
      QueueEntry entry;
      entry.edge = edge;
      entry.index = 0;
      entry.score = edge->ScoreOrBound(0);
      edges_.push(entry);
    }

    void FinishedAdding(Context &context) {}

    void More(const Score beat) {
      if (P::Bound() < beat) return;
      while (!edges_.empty()) {
        QueueEntry top(edges_.top());
        if (top.score < beat) {
          P::SetBound(top.score);
          return;
        }
        edges_.pop();
        if (Consider(beat, top)) {
          // New hypothesis was generated.  May not have gone lower than beat_.  
          P::SetBound(edges_.empty() ? -kScoreInf : edges_.top().score);
          return;
        }
      }
      P::SetEmpty();
    }

  private:
    // Return true if a new final_ was generated.  
    bool Consider(const Score beat, QueueEntry &top) {
      Option &edge = *top.edge;
      if (edge.Size() > top.index) {
        // Have a concrete hypothesis.
        const Final &got = edge[top.index];
        if (top.score != got.Score()) {
          top.score = got.Score();
          edges_.push(top);
          return false;
        }
        // Hypothesis matches the cached score.  Use it.  
        final_.push_back(&got);
        ++top.index;
        PushLower(top);
        return true;
      }
      // No concrete hypothesis.
      if (top.score != edge.Bound()) {
        PushBound(top);
        return false;
      }
      Score to_beat;
      if (edges_.empty()) {
        to_beat = beat;
      } else {
        to_beat = std::max(beat, edges_.top().score);
      }
      edge.More(to_beat);
      PushLower(top);
      return false;
    }

    void PushLower(QueueEntry &entry) {
      const Option &edge = *entry.edge;
      if (edge.Size() > entry.index) {
        entry.score = edge[entry.index].Score();
        edges_.push(entry);
        return;
      }
      PushBound(entry);
    }

    void PushBound(QueueEntry &entry) {
      entry.score = entry.edge->Bound();
      if (entry.score != -kScoreInf) edges_.push(entry);
    }

    std::priority_queue<QueueEntry> edges_;
};

} // namespace search
#endif // SEARCH_VERTEX__
