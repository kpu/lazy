#ifndef ALONE_READ__
#define ALONE_READ__

#include "util/exception.hh"

#include <iosfwd>

namespace util { class FilePiece; }

namespace search { class Context; }

namespace alone {

class Graph;

class FormatException : public util::Exception {
  public:
    FormatException() {}
    ~FormatException() throw() {}
};

void JustVocab(util::FilePiece &from, std::ostream &to);

bool ReadCDec(search::Context &context, util::FilePiece &from, Graph &to);

} // namespace alone

#endif // ALONE_READ__
