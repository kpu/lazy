#ifndef LM_MODEL__
#define LM_MODEL__

#include "lm/bhiksha.hh"
#include "lm/binary_format.hh"
#include "lm/config.hh"
#include "lm/facade.hh"
#include "lm/max_order.hh"
#include "lm/quantize.hh"
#include "lm/search_hashed.hh"
#include "lm/search_trie.hh"
#include "lm/value.hh"
#include "lm/vocab.hh"
#include "lm/weights.hh"

#include "util/murmur_hash.hh"

#include <algorithm>
#include <vector>

#include <string.h>

namespace util { class FilePiece; }

namespace lm {
namespace ngram {

// This is a POD but if you want memcmp to return the same as operator==, call
// ZeroRemaining first.    
class State {
  public:
    bool operator==(const State &other) const {
      if (length != other.length) return false;
      return !memcmp(words, other.words, length * sizeof(WordIndex));
    }

    // Three way comparison function.  
    int Compare(const State &other) const {
      if (length != other.length) return length < other.length ? -1 : 1;
      return memcmp(words, other.words, length * sizeof(WordIndex));
    }

    bool operator<(const State &other) const {
      if (length != other.length) return length < other.length;
      return memcmp(words, other.words, length * sizeof(WordIndex)) < 0;
    }

    // Call this before using raw memcmp.  
    void ZeroRemaining() {
      for (unsigned char i = length; i < kMaxOrder - 1; ++i) {
        words[i] = 0;
        backoff[i] = 0.0;
      }
    }

    unsigned char Length() const { return length; }

    // You shouldn't need to touch anything below this line, but the members are public so FullState will qualify as a POD.  
    // This order minimizes total size of the struct if WordIndex is 64 bit, float is 32 bit, and alignment of 64 bit integers is 64 bit.  
    WordIndex words[kMaxOrder - 1];
    float backoff[kMaxOrder - 1];
    unsigned char length;
};

inline uint64_t hash_value(const State &state, uint64_t seed = 0) {
  return util::MurmurHashNative(state.words, sizeof(WordIndex) * state.length, seed);
}

namespace detail {

// Should return the same results as SRI.  
// ModelFacade typedefs Vocabulary so we use VocabularyT to avoid naming conflicts.
template <class Search, class VocabularyT> class GenericModel : public base::ModelFacade<GenericModel<Search, VocabularyT>, State, VocabularyT> {
  private:
    typedef base::ModelFacade<GenericModel<Search, VocabularyT>, State, VocabularyT> P;
  public:
    // This is the model type returned by RecognizeBinary.
    static const ModelType kModelType;

    static const unsigned int kVersion = Search::kVersion;

    /* Get the size of memory that will be mapped given ngram counts.  This
     * does not include small non-mapped control structures, such as this class
     * itself.  
     */
    static size_t Size(const std::vector<uint64_t> &counts, const Config &config = Config());

    /* Load the model from a file.  It may be an ARPA or binary file.  Binary
     * files must have the format expected by this class or you'll get an
     * exception.  So TrieModel can only load ARPA or binary created by
     * TrieModel.  To classify binary files, call RecognizeBinary in
     * lm/binary_format.hh.  
     */
    explicit GenericModel(const char *file, const Config &config = Config());

    /* Score p(new_word | in_state) and incorporate new_word into out_state.
     * Note that in_state and out_state must be different references:
     * &in_state != &out_state.  
     */
    FullScoreReturn FullScore(const State &in_state, const WordIndex new_word, State &out_state) const;

    /* Slower call without in_state.  Try to remember state, but sometimes it
     * would cost too much memory or your decoder isn't setup properly.  
     * To use this function, make an array of WordIndex containing the context
     * vocabulary ids in reverse order.  Then, pass the bounds of the array:
     * [context_rbegin, context_rend).  The new_word is not part of the context
     * array unless you intend to repeat words.  
     */
    FullScoreReturn FullScoreForgotState(const WordIndex *context_rbegin, const WordIndex *context_rend, const WordIndex new_word, State &out_state) const;

    /* Get the state for a context.  Don't use this if you can avoid it.  Use
     * BeginSentenceState or EmptyContextState and extend from those.  If
     * you're only going to use this state to call FullScore once, use
     * FullScoreForgotState. 
     * To use this function, make an array of WordIndex containing the context
     * vocabulary ids in reverse order.  Then, pass the bounds of the array:
     * [context_rbegin, context_rend).  
     */
    void GetState(const WordIndex *context_rbegin, const WordIndex *context_rend, State &out_state) const;

    /* More efficient version of FullScore where a partial n-gram has already
     * been scored.  
     * NOTE: THE RETURNED .rest AND .prob ARE RELATIVE TO THE .rest RETURNED BEFORE.  
     */
    FullScoreReturn ExtendLeft(
        // Additional context in reverse order.  This will update add_rend to 
        const WordIndex *add_rbegin, const WordIndex *add_rend,
        // Backoff weights to use.  
        const float *backoff_in,
        // extend_left returned by a previous query.
        uint64_t extend_pointer,
        // Length of n-gram that the pointer corresponds to.  
        unsigned char extend_length,
        // Where to write additional backoffs for [extend_length + 1, min(Order() - 1, return.ngram_length)]
        float *backoff_out,
        // Amount of additional content that should be considered by the next call.
        unsigned char &next_use) const;

    /* Return probabilities minus rest costs for an array of pointers.  The
     * first length should be the length of the n-gram to which pointers_begin
     * points.  
     */
    float UnRest(const uint64_t *pointers_begin, const uint64_t *pointers_end, unsigned char first_length) const {
      // Compiler should optimize this if away.  
      return Search::kDifferentRest ? InternalUnRest(pointers_begin, pointers_end, first_length) : 0.0;
    }

  private:
    friend void lm::ngram::LoadLM<>(const char *file, const Config &config, GenericModel<Search, VocabularyT> &to);

    static void UpdateConfigFromBinary(int fd, const std::vector<uint64_t> &counts, Config &config);

    FullScoreReturn ScoreExceptBackoff(const WordIndex *const context_rbegin, const WordIndex *const context_rend, const WordIndex new_word, State &out_state) const;

    // Score bigrams and above.  Do not include backoff.   
    void ResumeScore(const WordIndex *context_rbegin, const WordIndex *const context_rend, unsigned char starting_order_minus_2, typename Search::Node &node, float *backoff_out, unsigned char &next_use, FullScoreReturn &ret) const;

    // Appears after Size in the cc file.
    void SetupMemory(void *start, const std::vector<uint64_t> &counts, const Config &config);

    void InitializeFromBinary(void *start, const Parameters &params, const Config &config, int fd);

    void InitializeFromARPA(const char *file, const Config &config);

    float InternalUnRest(const uint64_t *pointers_begin, const uint64_t *pointers_end, unsigned char first_length) const;

    Backing &MutableBacking() { return backing_; }

    Backing backing_;
    
    VocabularyT vocab_;

    Search search_;
};

} // namespace detail

// Instead of typedef, inherit.  This allows the Model etc to be forward declared.  
// Oh the joys of C and C++. 
#define LM_COMMA() ,
#define LM_NAME_MODEL(name, from)\
class name : public from {\
  public:\
    name(const char *file, const Config &config = Config()) : from(file, config) {}\
};

LM_NAME_MODEL(ProbingModel, detail::GenericModel<detail::HashedSearch<BackoffValue> LM_COMMA() ProbingVocabulary>);
LM_NAME_MODEL(RestProbingModel, detail::GenericModel<detail::HashedSearch<RestValue> LM_COMMA() ProbingVocabulary>);
LM_NAME_MODEL(TrieModel, detail::GenericModel<trie::TrieSearch<DontQuantize LM_COMMA() trie::DontBhiksha> LM_COMMA() SortedVocabulary>);
LM_NAME_MODEL(ArrayTrieModel, detail::GenericModel<trie::TrieSearch<DontQuantize LM_COMMA() trie::ArrayBhiksha> LM_COMMA() SortedVocabulary>);
LM_NAME_MODEL(QuantTrieModel, detail::GenericModel<trie::TrieSearch<SeparatelyQuantize LM_COMMA() trie::DontBhiksha> LM_COMMA() SortedVocabulary>);
LM_NAME_MODEL(QuantArrayTrieModel, detail::GenericModel<trie::TrieSearch<SeparatelyQuantize LM_COMMA() trie::ArrayBhiksha> LM_COMMA() SortedVocabulary>);

// Default implementation.  No real reason for it to be the default.  
typedef ::lm::ngram::ProbingVocabulary Vocabulary;
typedef ProbingModel Model;

} // namespace ngram
} // namespace lm

#endif // LM_MODEL__
