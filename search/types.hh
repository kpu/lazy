#ifndef SEARCH_TYPES__
#define SEARCH_TYPES__

#include <stdint.h>

namespace lm { namespace ngram { class ChartState; } }

namespace search {

typedef float Score;

typedef uint32_t Arity;

union Note {
  const void *vp;
};

typedef void *History;

struct NBestComplete {
  History history;
  const lm::ngram::ChartState *state;
  Score score;
};

} // namespace search

#endif // SEARCH_TYPES__
