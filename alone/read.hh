#ifndef ALONE_READ__
#define ALONE_READ__

#include "util/exception.hh"

#include <iosfwd>

namespace util { class FilePiece; }

namespace search {
template <class Model> class Context;
class EdgeGenerator;
} // namespace search

namespace alone {

class Graph;
class Vocab;

class FormatException : public util::Exception {
  public:
    FormatException() {}
    ~FormatException() throw() {}
};

void ReadGraphCounts(util::FilePiece &from, Graph &graph);

template <class Model> void ReadEdges(search::Context<Model> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &edges);

} // namespace alone

#endif // ALONE_READ__
