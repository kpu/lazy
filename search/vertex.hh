#ifndef SEARCH_VERTEX__
#define SEARCH_VERTEX__

#include "search/types.hh"

#include <queue>
#include <vector>

namespace search {

template <class Final> class Edge;

template <class Final> class Vertex {
  private:
    struct QueueEntry {
      Edge<Final> *edge;
      Index index;
      // Cached score.  
      Score score;
      // Priority queue's order is annoying.  
      bool operator<(const QueueEntry &other) const {
        return score > other.score;
      }
    };

  public:
    Vertex() : bound_(kScoreInf) {}

    void AddEdge(Edge *edge) {
      QueueEntry entry;
      entry.edge = edge;
      entry.index = 0;
      entry.score = kScoreInf;
      edges_.push(entry);
    }

    Index Size() const {
      return final_.size();
    }

    const Final &operator[](Index i) const {
      return *final_[i];
    }

    Score Bound() const {
      return bound_;
    }

    void More(const Score beat) {
      if (bound_ < beat) return;
      while (!edges_.empty()) {
        QueueEntry top(edges_.top());
        if (top.score < beat) {
          bound_ = top.score;
          return;
        }
        edges_.pop();
        if (Consider(beat, top)) {
          // New hypothesis was generated.  May not have gone lower than beat_.  
          bound_ = edges_.empty() ? -kScoreInf : edges_.top().score;
          return;
        }
      }
      bound_ = -kScoreInf;
    }

  private:
    // Return true if a new final_ was generated.  
    bool Consider(const Score beat, QueueEntry &top) {
      Edge &edge = *top.edge;
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
      const Edge &edge = *entry.edge;
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

    std::vector<const Final*> final_;

    Score bound_;

    std::priority_queue<QueueEntry> edges_;
};

} // namespace search
#endif // SEARCH_VERTEX__
