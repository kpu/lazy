#ifndef SEARCH_NBEST__
#define SEARCH_NBEST__

#include "search/applied.hh"
#include "search/edge.hh"

#include <boost/pool/object_pool.hpp>

#include <cstddef>
#include <queue>
#include <vector>

#include <assert.h>

namespace search {

class NBestList;

class NBestList {
  private:
    class RevealedRef {
      public: 
        explicit RevealedRef(History history) 
          : in_(static_cast<NBestList*>(history)), index_(0) {}

      private:
        friend class NBestList;

        NBestList *in_;
        std::size_t index_;
    };
    
    typedef GenericApplied<RevealedRef> QueueEntry;

  public:
    NBestList(std::vector<PartialEdge> &existing, util::Pool &entry_pool, std::size_t keep);

    Score TopAfterConstructor() const;

    Score Visit(util::Pool &pool, std::size_t index);

    Applied Get(util::Pool &pool, std::size_t index) {
      assert(index <= revealed_.size());
      if (index == revealed_.size()) MoveTop(pool);
      return revealed_[index];
    }

  private:
    void MoveTop(util::Pool &pool);

    typedef std::vector<Applied> Revealed;
    Revealed revealed_;

    typedef std::priority_queue<QueueEntry> Queue;
    Queue queue_;
};

class NBest {
  public:
    typedef std::vector<PartialEdge> Combine;

    explicit NBest(std::size_t keep) : keep_(keep) {}

    void Add(std::vector<PartialEdge> &existing, PartialEdge addition) const {
      existing.push_back(addition);
    }

    NBestComplete Complete(std::vector<PartialEdge> &partials);

  private:
    const std::size_t keep_;

    boost::object_pool<NBestList> list_pool_;

    util::Pool entry_pool_;
};

} // namespace search

#endif // SEARCH_NBEST__
