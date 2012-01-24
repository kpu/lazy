#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/types.hh"

#include <queue>
#include <vector>

namespace search {

class Vertex;

template <class Rule> class Edge {
  private:
    struct QueueEntry {
      Index *indices;
      float score;

      bool operator<(const QueueEntry &other) const {
        return score > other.score;
      }
    };

  public:
    typedef typename Rule::Final Final;

    explicit Edge(const Rule &rule) 
      : bound_(kScoreInf), rule_(rule), index_pool_(sizeof(Index) * rule_.Variables()) {}

    Index Size() const {
      return final_.size();
    }

    const Final &operator[](Index i) const {
      return final_[i];
    }

    Score Bound() const {
      return bound_;
    }

    void More(const Score beat) {
      if (bound_ < beat) return;
      // Add hypotheses to holding tank. 
      

      bound_ = generate_.empty() ? -kScoreInf : generate_.top().score;
      // Move hypotheses from holding tank to final.   
      while (!holding_.empty()) {
        const Final &top = holding_.top();
        if (top.Score() < bound_) break;
        final_.push_back(top);
        holding_.pop();
      }
    }

  private:
    std::vector<Final> final_;

    Score bound_;

    struct FinalGreater : public std::binary_function<const Final &, const Final &, bool> {
      bool operator()(const Final &first, const Final &second) const {
        return final.Score() > second.Score();
      }
    };

    std::priority_queue<Final, std::vector<Final>, FinalGreater> holding_;

    // Rule and pointers to rule arguments.  
    Rule rule_;
    std::vector<Vertex*> to_;

    boost::pool<> index_pool_;

    std::priority_queue<QueueEntry> generate_;
};

} // namespace search
#endif // SEARCH_EDGE__
