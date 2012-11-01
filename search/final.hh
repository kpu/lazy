#ifndef SEARCH_FINAL__
#define SEARCH_FINAL__

#include "search/edge.hh"
#include "search/header.hh"
#include "util/pool.hh"

namespace search {

// A full hypothesis with pointers to children.
class Final : public Header {
  public:
    Final() {}

    Final(util::Pool &pool, PartialEdge partial) 
      : Header(pool.Allocate(Size(partial.GetArity()))) {
      memcpy(Base(), partial.Base(), kHeaderSize);
      Final *child_out = Children();
      const PartialVertex *part = partial.NT();
      const PartialVertex *const part_end_loop = part + partial.GetArity();
      for (; part != part_end_loop; ++part, ++child_out)
        *child_out = Final(part->End());
    }

    explicit Final(History from) : Header(from) {}

    History AsHistory() {
      return Base();
    }

    // These are arrays of length GetArity().
    Final *Children() {
      return reinterpret_cast<Final*>(After());
    }
    const Final *Children() const {
      return reinterpret_cast<const Final*>(After());
    }

  private:
    static std::size_t Size(Arity arity) {
      return kHeaderSize + arity * sizeof(const Final);
    }
};

class SingleBest {
  public:
    typedef PartialEdge Combine;

    void Add(PartialEdge &existing, PartialEdge add) const {
      if (!existing.Valid() || existing.GetScore() < add.GetScore())
        existing = add;
    }

    NBestComplete Complete(PartialEdge partial) {
      NBestComplete ret;
      ret.history = Final(pool_, partial).AsHistory();
      ret.state = &partial.CompletedState();
      ret.score = partial.GetScore();
      return ret;
    }

  private:
    util::Pool pool_;
};

} // namespace search

#endif // SEARCH_FINAL__
