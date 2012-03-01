#ifndef SEARCH_CONTEXT__
#define SEARCH_CONTEXT__

#include "search/types.hh"
#include "util/exception.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace search {

class PoolOut : public util::Exception {
  public:
    PoolOut() throw() {
      *this << "Pool returned NULL";
    }

    ~PoolOut() throw() {}
};

template <class Final> class Context {
  public:
    template <class Child, class R> Final *ApplyRule(Child &child_class, const R &rule, const typename Final::ChildArray &children) {
      Final *ret = final_pool_.construct(child_class, rule, children);
      UTIL_THROW_IF(!ret, PoolOut, " for finals");
      return ret;
    }

    void DeleteFinal(Final *final) {
      final_pool_.destroy(final);
    }

  private:
    boost::object_pool<Final> final_pool_;
};

} // namespace search

#endif // SEARCH_CONTEXT__
