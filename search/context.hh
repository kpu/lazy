#ifndef SEARCH_CONTEXT__
#define SEARCH_CONTEXT__

#include <boost/pool/object_pool.hpp>

#include <vector>

namespace search {

template <class Final> class Context {
  public:
    template <class R> Final *ApplyRule(const R &rule, std::vector<const Final *> &values) {
      Final *ret = final_pool_.construct(*this, rule, values);
      if (!ret) throw std::bad_alloc();
      return ret;
    }

    void DeleteFinal(Final *final) {
      final_pool_.destroy(final);
    }

    std::vector<const Final*> &ClearedTemp() {
      have_values_.clear();
      return have_values_;
    }

  private:
    boost::object_pool<Final> final_pool_;

    // Temporary used internally by edges.  
    std::vector<const Final*> have_values_;
};

} // namespace search

#endif // SEARCH_CONTEXT__
