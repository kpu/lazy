#ifndef ALONE_ASSEMBLE__
#define ALONE_ASSEMBLE__

#include <iosfwd>

namespace search {
class Final;
} // namespace search

namespace alone {

std::ostream &operator<<(std::ostream &o, const search::Final &final);

void DetailedFinal(std::ostream &o, const search::Final &final, const char *indent_str = "  ");

} // namespace alone

#endif // ALONE_ASSEMBLE__
