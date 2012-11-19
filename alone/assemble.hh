#ifndef ALONE_ASSEMBLE__
#define ALONE_ASSEMBLE__

#include <iosfwd>
#include <vector>

namespace search { class Applied; }

namespace alone {

std::ostream &JustText(std::ostream &to, const search::Applied final);

void DetailedFinal(std::ostream &o, const search::Applied final, const char *indent_str = "  ");

} // namespace alone

#endif // ALONE_ASSEMBLE__
