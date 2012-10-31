#ifndef SEARCH_TYPES__
#define SEARCH_TYPES__

#include <stdint.h>

namespace search {

typedef float Score;

typedef uint32_t Arity;

union Note {
  const void *vp;
};

typedef void *History;

} // namespace search

#endif // SEARCH_TYPES__
