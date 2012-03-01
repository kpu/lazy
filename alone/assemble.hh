#ifndef ALONE_ASSEMBLE__
#define ALONE_ASSEMBLE__

#include <iosfwd>

namespace alone {

class Final;

std::ostream &operator<<(std::ostream &o, const Final &final);

} // namespace alone

#endif // ALONE_ASSEMBLE__
