#ifndef ALONE_ASSEMBLE__
#define ALONE_ASSEMBLE__

#include <iosfwd>

namespace search { class Applied; }

namespace alone {

namespace feature { class WeightsBase; }

std::ostream &JustText(std::ostream &to, const search::Applied final);

std::ostream &SingleLine(std::ostream &o, const search::Applied final, const feature::WeightsBase &dictionary);

void DetailedFinal(std::ostream &o, const search::Applied final, const char *indent_str = "  ");

} // namespace alone

#endif // ALONE_ASSEMBLE__
