#ifndef ALONE_READ__
#define ALONE_READ__

#include "util/exception.hh"

#include <iosfwd>

namespace util { class FilePiece; }

namespace search {
template <class Model> class Context;
class SingleBest;
} // namespace search

namespace alone {

class Graph;
class Vocab;

class FormatException : public util::Exception {
  public:
    FormatException() {}
    ~FormatException() throw() {}
};

template <class Model> void ReadCDec(search::Context<Model> &context, util::FilePiece &from, Graph &to, Vocab &vocab, search::SingleBest &best);

} // namespace alone

#endif // ALONE_READ__
