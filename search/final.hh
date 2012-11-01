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

    Final(util::Pool &pool, Score score, Arity arity, Note note) 
      : Header(pool.Allocate(Size(arity)), arity) {
      SetScore(score);
      SetNote(note);
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
      Final final(pool_, partial.GetScore(), partial.GetArity(), partial.GetNote());
      Final *child_out = final.Children();
      const PartialVertex *part = partial.NT();
      const PartialVertex *const part_end_loop = part + partial.GetArity();
      for (; part != part_end_loop; ++part, ++child_out)
        *child_out = Final(part->End());
      NBestComplete ret;
      ret.history = final.AsHistory();
      ret.state = &partial.CompletedState();
      ret.score = partial.GetScore();
      return ret;
    }

  private:
    util::Pool pool_;
};

} // namespace search

#endif // SEARCH_FINAL__
