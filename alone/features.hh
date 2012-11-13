// Handling for individual feature values.  
// Feature vectors are represented sparsely as a vector pairing id and value.
#ifndef ALONE_FEATURES__
#define ALONE_FEATURES__

#include "search/types.hh"
#include "util/exception.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/unordered_map.hpp>

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <queue>
#include <string>
#include <vector>

#include <stdint.h>

namespace util { class FilePiece; }

namespace alone {
namespace feature {

typedef uint32_t ID;

class Vector {
  public:
    Vector() {}

  private:
    friend class Adder;
    friend class WeightsBase;
    friend class Test;

    struct Entry {
      ID id;
      search::Score score;

      // sorting order inside values_.  
      bool operator<(const Entry &other) const {
        return id < other.id;
      }
    };

    std::vector<Entry> values_;
};

// Sum a bunch of Vector at once.  
class Adder {
  public:
    void Add(const Vector &vec) {
      if (vec.values_.empty()) return;
      Part part;
      part.cur = &*vec.values_.begin();
      part.end = part.cur + vec.values_.size();
      queue_.push(part);
    }

    void Finish(Vector &out);

  private:
    struct Part {
      const Vector::Entry *cur, *end;
      bool operator>(const Part &other) const {
        return cur->id > other.cur->id;
      }
    };

    std::priority_queue<Part, std::vector<Part>, std::greater<Part> > queue_;
};

class WeightParseException : public util::Exception {
  public:
    WeightParseException() {}
    ~WeightParseException() throw() {}
};

// Integrated weight and string identification.  Why?  One hash table lookup
// for parsing.  
class WeightsBase {
  public:
    explicit WeightsBase(util::FilePiece &f);

    explicit WeightsBase(StringPiece str);

    // Parse a feature vector, returning the dot product with eights.
    search::Score Parse(StringPiece from, Vector &to);

    std::ostream &Write(std::ostream &to, const Vector &from) const;

  protected:
    // Lookup a feature by name and complain if it can't be found.  
    search::Score Lookup(StringPiece name, std::ostream *complain = NULL) const;

  private:
    ID Add(StringPiece str, search::Score weight);

    // owns strings behind the StringPieces.  
    util::Pool pool_;
    // id to string lookup
    std::vector<StringPiece> id_;

    // string to id lookup
    struct Value {
      ID id;
      search::Score weight;
    };
    typedef boost::unordered_map<StringPiece, Value> Map;
    Map str_;
};

class Weights : public WeightsBase {
  public:
    // If complain is non-null then write a complaint for any implicitly zero
    // hard-coded feature.  
    explicit Weights(util::FilePiece &f, std::ostream *complain = NULL);

    explicit Weights(StringPiece str, std::ostream *complain = NULL);

    search::Score LM() const { return lm_; }
    search::Score OOV() const { return oov_; }
    search::Score WordPenalty() const { return word_penalty_; }

    const search::Weights &GetSearch() const { return search_; }

  private:
    search::Score lm_, oov_, word_penalty_;
};

} // namespace feature
} // namespace alone

#endif // ALONE_FEATURES__
