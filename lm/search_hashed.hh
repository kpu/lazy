#ifndef LM_SEARCH_HASHED__
#define LM_SEARCH_HASHED__

#include "lm/model_type.hh"
#include "lm/config.hh"
#include "lm/read_arpa.hh"
#include "lm/return.hh"
#include "lm/weights.hh"

#include "util/bit_packing.hh"
#include "util/probing_hash_table.hh"

#include <algorithm>
#include <iostream>
#include <vector>

namespace util { class FilePiece; }

namespace lm {
namespace ngram {
struct Backing;
namespace detail {

inline uint64_t CombineWordHash(uint64_t current, const WordIndex next) {
  uint64_t ret = (current * 8978948897894561157ULL) ^ (static_cast<uint64_t>(1 + next) * 17894857484156487943ULL);
  return ret;
}

class ValuePointer {
  public:
    explicit ValuePointer(const ProbBackoff &to) : to_(&to) {}

    ValuePointer() : to_(NULL) {}

    bool Found() const {
      return to_ != NULL;
    }

    float Prob() const {
      util::FloatEnc enc;
      enc.f = to_->prob;
      enc.i |= util::kSignBit;
      return enc.f;
    }

    float Backoff() const {
      return to_->backoff;
    }

    bool IndependentLeft() const {
      util::FloatEnc enc;
      enc.f = to_->prob;
      return enc.i & util::kSignBit;
    }

  private:
    const ProbBackoff *to_;
};

class LongestPointer {
  public:
    explicit LongestPointer(const float &to) : to_(&to) {}

    LongestPointer() : to_(NULL) {}

    bool Found() const {
      return to_ != NULL;
    }

    float Prob() const {
      return *to_;
    }

  private:
    const float *to_;
};

class HashedSearch {
  public:
    typedef uint64_t Node;

    class Unigram {
      public:
        Unigram() {}

        Unigram(void *start, std::size_t /*allocated*/) : unigram_(static_cast<ProbBackoff*>(start)) {}

        static std::size_t Size(uint64_t count) {
          return (count + 1) * sizeof(ProbBackoff); // +1 for hallucinate <unk>
        }

        const ProbBackoff &Lookup(WordIndex index) const { return unigram_[index]; }

        ProbBackoff &Unknown() { return unigram_[0]; }

        void LoadedBinary() {}

        // For building.
        ProbBackoff *Raw() { return unigram_; }

      private:
        ProbBackoff *unigram_;
    };


    typedef ValuePointer UnigramPointer;
    typedef ValuePointer MiddlePointer;
    typedef ::lm::ngram::detail::LongestPointer LongestPointer;

    ProbBackoff &UnknownUnigram() { return unigram_.Unknown(); }

    ValuePointer LookupUnigram(WordIndex word, Node &next, bool &independent_left, uint64_t &extend_left) const {
      extend_left = static_cast<uint64_t>(word);
      next = extend_left;
      ValuePointer ret(unigram_.Lookup(word));
      independent_left = ret.IndependentLeft();
      return ret;
    }

  protected:
    Unigram unigram_;
};

template <class Middle, class Longest> class TemplateHashedSearch : public HashedSearch {
  public:
    static const unsigned int kVersion = 0;

    // TODO: move probing_multiplier here with next binary file format update.  
    static void UpdateConfigFromBinary(int, const std::vector<uint64_t> &, Config &) {}

    static std::size_t Size(const std::vector<uint64_t> &counts, const Config &config) {
      std::size_t ret = Unigram::Size(counts[0]);
      for (unsigned char n = 1; n < counts.size() - 1; ++n) {
        ret += Middle::Size(counts[n], config.probing_multiplier);
      }
      return ret + Longest::Size(counts.back(), config.probing_multiplier);
    }

    uint8_t *SetupMemory(uint8_t *start, const std::vector<uint64_t> &counts, const Config &config);

    template <class Voc> void InitializeFromARPA(const char *file, util::FilePiece &f, const std::vector<uint64_t> &counts, const Config &config, Voc &vocab, Backing &backing);

    void LoadedBinary();

    unsigned char Order() const {
      return middle_.size() + 2;
    }

#pragma GCC diagnostic ignored "-Wuninitialized"
    ValuePointer Unpack(uint64_t extend_pointer, unsigned char extend_length, Node &node) const {
      node = extend_pointer;
      typename Middle::ConstIterator found;
      bool got = middle_[extend_length - 2].Find(extend_pointer, found);
      assert(got);
      (void)got;
      return ValuePointer(found->value);
    }

    ValuePointer LookupMiddle(unsigned char order_minus_2, WordIndex word, Node &node, bool &independent_left, uint64_t &extend_pointer) const {
      node = CombineWordHash(node, word);
      typename Middle::ConstIterator found;
      if (!middle_[order_minus_2].Find(node, found)) {
        independent_left = true;
        return ValuePointer();
      }
      extend_pointer = node;
      ValuePointer ret(found->value);
      independent_left = ret.IndependentLeft();
      return ret;
    }

    LongestPointer LookupLongest(WordIndex word, const Node &node) const {
      // Sign bit is always on because longest n-grams do not extend left.  
      typename Longest::ConstIterator found;
      if (!longest_.Find(CombineWordHash(node, word), found)) return LongestPointer();
      return LongestPointer(found->value.prob);
    }

    // Generate a node without necessarily checking that it actually exists.  
    // Optionally return false if it's know to not exist.  
    bool FastMakeNode(const WordIndex *begin, const WordIndex *end, Node &node) const {
      assert(begin != end);
      node = static_cast<Node>(*begin);
      for (const WordIndex *i = begin + 1; i < end; ++i) {
        node = CombineWordHash(node, *i);
      }
      return true;
    }

  private:
    std::vector<Middle> middle_;
    Longest longest_;
};

/* These look like perfect candidates for a template, right?  Ancient gcc (4.1
 * on RedHat stale linux) doesn't pack templates correctly.  ProbBackoffEntry
 * is a multiple of 8 bytes anyway.  ProbEntry is 12 bytes so it's set to pack.
 */
struct ProbBackoffEntry {
  uint64_t key;
  ProbBackoff value;
  typedef uint64_t Key;
  typedef ProbBackoff Value;
  uint64_t GetKey() const {
    return key;
  }
};

#pragma pack(push)
#pragma pack(4)
struct ProbEntry {
  uint64_t key;
  Prob value;
  typedef uint64_t Key;
  typedef Prob Value;
  uint64_t GetKey() const {
    return key;
  }
};

#pragma pack(pop)

struct ProbingHashedSearch : public TemplateHashedSearch<
  util::ProbingHashTable<ProbBackoffEntry, util::IdentityHash>,
  util::ProbingHashTable<ProbEntry, util::IdentityHash> > {

  static const ModelType kModelType = HASH_PROBING;
};

} // namespace detail
} // namespace ngram
} // namespace lm

#endif // LM_SEARCH_HASHED__
