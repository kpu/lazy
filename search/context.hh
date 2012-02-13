#ifndef SEARCH_CONTEXT__
#define SEARCH_CONTEXT__

#include "search/types.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace search {

template <class Final> class Context {
  public:
    template <class Child, class R> Final *ApplyRule(Child &child_class, const R &rule, std::vector<const Final *> &values) {
      Final *ret = final_pool_.construct(child_class, rule, values);
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

    void EnsureIndexPool(Index arity) {
      while (index_pools_.size() <= arity) {
        index_pools_.push_back(new boost::pool<>(index_pools_.size() * sizeof(Index)));
      }
    }

    Index *NewIndices(Index arity) {
      assert(index_pools_.size() > arity);
      Index *ret = static_cast<Index*>(index_pools_[arity].malloc());
      if (!ret) throw std::bad_alloc();
      return ret;
    }

    void DeleteIndices(Index arity, Index *ptr) {
      index_pools_[arity].free(ptr);
    }

  private:
    boost::object_pool<Final> final_pool_;

    boost::ptr_vector<boost::pool<> > index_pools_;

    // Temporary used internally by edges.  
    std::vector<const Final*> have_values_;
};

} // namespace search

#endif // SEARCH_CONTEXT__
