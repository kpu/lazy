#include "search/edge.hh"

#define BOOST_TEST_MODULE EdgeTest
#include <boost/test/unit_test.hpp>

namespace search {
namespace {

struct FakeFinal {
  Score Score() const {
    return 3.14;
  }
};

struct FakeRule {
  typedef FakeFinal Final;

  Index Variables() const { return 0; }
  void Apply(const std::vector<const Final*> &values, Final *to) const {}
};

BOOST_AUTO_TEST_CASE(empty) {
  Edge<FakeRule> edge(FakeRule());
  Context<FakeFinal> context;
  edge.FinishedAdding();
  BOOST_EXPECT_EQUAL(3.14, edge.Score());
}

}
} // namespace search
