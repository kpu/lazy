#include "search/vertex.hh"

#include <algorithm>
#include <functional>

#include <assert.h>

namespace search {

namespace {

struct GreaterByBound : public std::binary_function<const VertexNode *, const VertexNode *, bool> {
  bool operator()(const VertexNode *first, const VertexNode *second) const {
    return first->Bound() > second->Bound();
  }
};

} // namespace

void VertexNode::SortAndSet() {
  if (Complete()) {
    assert(end_);
    assert(extend_.empty());
    bound_ = end_->Bound();
    return;
  }
  for (std::vector<VertexNode*>::const_iterator i = extend_.begin(); i != extend_.end(); ++i) {
    (*i)->SortAndSet();
  }
  std::sort(extend_.begin(), extend_.end(), GreaterByBound());
  bound_ = extend_.front()->Bound();
}

namespace {
VertexNode kBlankVertexNode;
} // namespace

PartialVertex kBlankPartialVertex(kBlankVertexNode);

} // namespace search
