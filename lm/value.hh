#ifndef LM_VALUE__
#define LM_VALUE__

#include "lm/model_type.hh"
#include "lm/weights.hh"

namespace lm {
namespace ngram {

// Template proxy for probing unigrams and middle.
template <class Weights> class GenericProbingProxy {
  public:
    explicit GenericProbingProxy(const Weights &to) : to_(&to) {}

    GenericProbingProxy() : to_(NULL) {}

    bool Found() const { return to_ != NULL; }

    float Prob() const {
      util::FloatEnc enc;
      enc.f = to_->prob;
      enc.i |= util::kSignBit;
      return enc.f;
    }

    float Backoff() const { return to_->backoff; }

    bool IndependentLeft() const {
      util::FloatEnc enc;
      enc.f = to_->prob;
      return enc.i & util::kSignBit;
    }

  protected:
    const Weights *to_;
};

// Basic proxy for trie unigrams.  
template <class Weights> class GenericTrieUnigramProxy {
  public:
    explicit GenericTrieUnigramProxy(const Weights &to) : to_(&to) {}

    GenericTrieUnigramProxy() : to_(NULL) {}

    bool Found() const { return to_ != NULL; }
    float Prob() const { return to_->prob; }
    float Backoff() const { return to_->backoff; }
    float Rest() const { return Prob(); }

  protected:
    const Weights *to_;
};

struct BackoffValue {
  typedef ProbBackoff Weights;
  static const ModelType kProbingModelType = PROBING;

  class ProbingProxy : public GenericProbingProxy<Weights> {
    public:
      explicit ProbingProxy(const Weights &to) : GenericProbingProxy<Weights>(to) {}
      ProbingProxy() {}
      float Rest() const { return Prob(); }
  };

  class TrieUnigramProxy : public GenericTrieUnigramProxy<Weights> {
    public:
      explicit TrieUnigramProxy(const Weights &to) : GenericTrieUnigramProxy<Weights>(to) {}
      TrieUnigramProxy() {}
      float Rest() const { return Prob(); }
  };

  struct ProbingEntry {
    typedef uint64_t Key;
    typedef Weights Value;
    uint64_t key;
    ProbBackoff value;
    uint64_t GetKey() const { return key; }
  };

  struct TrieUnigramValue {
    Weights weights;
    uint64_t next;
    uint64_t Next() const { return next; }
  };

  static void SetRest(const Prob &/*prob*/) {}
  static void SetRest(const Weights &/*weights*/) {}
  static bool MarkExtends(Weights &weights, const Weights &/*from*/) {
    util::UnsetSign(weights.prob);
    return false;
  }

  static bool MarkExtends(Weights &weights, const Prob &/*from*/) {
    util::UnsetSign(weights.prob);
    return false;
  }

  // Probing doesn't need to go back to unigram.
  const static bool kMarkEvenLower = false;
};

struct RestValue {
  typedef RestWeights Weights;
  static const ModelType kProbingModelType = REST_PROBING;

  class ProbingProxy : public GenericProbingProxy<RestWeights> {
    public:
      explicit ProbingProxy(const Weights &to) : GenericProbingProxy<RestWeights>(to) {}
      ProbingProxy() {}
      float Rest() const { return to_->rest; }
  };

  class TrieUnigramProxy : public GenericTrieUnigramProxy<Weights> {
    public:
      explicit TrieUnigramProxy(const Weights &to) : GenericTrieUnigramProxy<Weights>(to) {}
      TrieUnigramProxy() {}
      float Rest() const { return to_->rest; }
  };

// gcc 4.1 doesn't properly back dependent types :-(.  
#pragma pack(push)
#pragma pack(4)
  struct ProbingEntry {
    typedef uint64_t Key;
    typedef Weights Value;
    Key key;
    Value value;
    Key GetKey() const { return key; }
  };

  struct TrieUnigramValue {
    Weights weights;
    uint64_t next;
    uint64_t Next() const { return next; }
  };
#pragma pack(pop)

  static void SetRest(const Prob &/*prob*/) {}
  static void SetRest(Weights &weights) {
    weights.rest = weights.prob;
    util::SetSign(weights.rest);
  }

  static bool MarkExtends(Weights &weights, const Weights &from) {
    util::UnsetSign(weights.prob);
    if (weights.rest >= from.rest) return false;
    weights.rest = from.rest;
    return true;
  }

  static bool MarkExtends(Weights &weights, const Prob &from) {
    util::UnsetSign(weights.prob);
    if (weights.rest >= from.prob) return false;
    weights.rest = from.prob;
    return true;
  }


  // Probing does need to go back to unigram.  
  const static bool kMarkEvenLower = true;
};

} // namespace ngram
} // namespace lm

#endif // LM_VALUE__
