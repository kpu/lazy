#include "search/edge.hh"

#define BOOST_TEST_MODULE EdgeTest
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace search {
namespace {

struct FakeFinal {
  Score Total() const {
    return 3.14;
  }
};

struct FakeRule {
  typedef FakeFinal Final;

  Index Variables() const { return 0; }

  Score Bound() const { return 5.78; }

  void Apply(const std::vector<const Final*> &values, Final *to) const {}
};

BOOST_AUTO_TEST_CASE(empty) {
  FakeRule fake;
  Edge<FakeRule> edge(fake);
  Context<FakeFinal> context;
  edge.FinishedAdding(context);
  BOOST_REQUIRE_EQUAL(1, edge.Size());
  BOOST_CHECK_CLOSE(3.14, edge[0].Total(), 0.0001);
  BOOST_CHECK_EQUAL(-kScoreInf, edge.Bound());
}

}
} // namespace search
