#ifndef ALONE_ASSEMBLE__
#define ALONE_ASSEMBLE__

#include "lm/word_index.hh"

#include <iosfwd>
#include <vector>

namespace search { class Applied; }

namespace alone {
namespace feature { class Vector; }

std::ostream &JustText(std::ostream &to, const search::Applied final);

void ComputeForFeatures(const search::Applied final, std::vector<lm::WordIndex> &words, feature::Vector &stored);

void DetailedFinal(std::ostream &o, const search::Applied final, const char *indent_str = "  ");

} // namespace alone

#endif // ALONE_ASSEMBLE__
