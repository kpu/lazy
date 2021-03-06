#ifndef ALONE_VOCAB__
#define ALONE_VOCAB__

#include "lm/word_index.hh"
#include "util/murmur_hash.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/functional/hash/hash.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace lm { namespace base { class Vocabulary; } }

namespace alone {

class Vocab {
  public:
    explicit Vocab(const lm::base::Vocabulary &backing);

    typedef std::pair<const char *const, lm::WordIndex> Entry;

    const Entry &FindOrAdd(const StringPiece &str);

  private:
    util::Pool piece_backing_;

    struct Hash : public std::unary_function<const char *, std::size_t> {
      std::size_t operator()(StringPiece str) const {
        return util::MurmurHashNative(str.data(), str.size());
      }
    };

    struct Equals : public std::binary_function<const char *, const char *, bool> {
      bool operator()(StringPiece first, StringPiece second) const {
        return first == second;
      }
    };

    typedef boost::unordered_map<const char *, lm::WordIndex, Hash, Equals> Map;
    Map map_;

    const lm::base::Vocabulary &backing_;
};

} // namespace alone
#endif // ALONE_VCOAB__
