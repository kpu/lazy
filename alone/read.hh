#ifndef ALONE_READ__
#define ALONE_READ__

#include "util/exception.hh"

namespace util { class FilePiece; }

namespace alone {

class Context;
class Graph;

class FormatException : public util::Exception {
  public:
    FormatException() {}
    ~FormatException() throw() {}
};

void ReadCDec(Context &context, util::FilePiece &from, Graph &to);

} // namespace alone

#endif // ALONE_READ__
