#ifndef SEARCH_VERTEX__
#define SEARCH_VERTEX__

#include "search/source.hh"
#include "search/types.hh"

#include <queue>
#include <vector>

namespace search {

// Child is typically an instantiation of Edge.
template <class Child> class Vertex : public Source<typename Child::Final> {
  public:
    typedef typename Child::Final Final;

  private:
    typedef Source<Final> P;

    struct QueueEntry {
      Child *edge;
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

    void Add(Child &edge) {
      QueueEntry entry;
      entry.edge = &edge;
      entry.index = 0;
      entry.score = edge.ScoreOrBound(0);
      if (entry.score != -kScoreInf) edges_.push(entry);
    }

    void FinishedAdding() {
      SetBound(edges_.empty() ? -kScoreInf : edges_.top().score);
    }

    void More(Context<Final> &context, const Score beat) {
      if (P::Bound() < beat) return;
      while (!edges_.empty()) {
        QueueEntry top(edges_.top());
        if (top.score < beat) {
          P::SetBound(top.score);
          return;
        }
        edges_.pop();
        if (Consider(context, beat, top)) {
          // New hypothesis was generated.  May not have gone lower than beat_.  
          P::SetBound(edges_.empty() ? -kScoreInf : edges_.top().score);
          return;
        }
      }
      P::SetBound(-kScoreInf);
    }

  private:
    // Return true if a new final_ was generated.  
    bool Consider(Context<Final> &context, const Score beat, QueueEntry &top) {
      Child &edge = *top.edge;
      if (edge.Size() > top.index) {
        // Have a concrete hypothesis.
        const Final &got = edge[top.index];
        if (top.score != got.Total()) {
          top.score = got.Total();
          edges_.push(top);
          return false;
        }
        // Hypothesis matches the cached score.  Use it.  
        AddFinal(got);
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
      edge.More(context, to_beat);
      PushLower(top);
      return false;
    }

    void PushLower(QueueEntry &entry) {
      const Child &edge = *entry.edge;
      if (edge.Size() > entry.index) {
        entry.score = edge[entry.index].Total();
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
