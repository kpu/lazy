#include "search/context.hh"
#include "search/edge.hh"

#define BOOST_TEST_MODULE EdgeTest
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <vector>

namespace search {
namespace {

class ZeroRule;
class MockRule;
struct FakeFinal {
  FakeFinal(const Context<FakeFinal> &, const ZeroRule &, std::vector<const FakeFinal *> &values);
  FakeFinal(const Context<FakeFinal> &, const MockRule &, std::vector<const FakeFinal *> &values);

  Score Total() const {
    return 3.14;
  }

  uint64_t RecombineHash() const {
    return 10;
  }

  void Recombine(Context<FakeFinal> &context, FakeFinal *with) {
    context.DeleteFinal(with);
  }
};

struct ZeroRule {
  typedef FakeFinal Final;

  Index Variables() const { return 0; }

  Score Bound() const { return 5.78; }
};

FakeFinal::FakeFinal(const Context<FakeFinal> &, const ZeroRule &rule, std::vector<const FakeFinal*> &values) {
  BOOST_CHECK_EQUAL(rule.Variables(), values.size());
}

BOOST_AUTO_TEST_CASE(empty) {
  Edge<ZeroRule> edge;
  Context<FakeFinal> context;
  edge.FinishedAdding(context);
  BOOST_REQUIRE_EQUAL(1, edge.Size());
  BOOST_CHECK_CLOSE(3.14, edge[0].Total(), 0.0001);
  BOOST_CHECK_EQUAL(-kScoreInf, edge.Bound());
}

struct MockRule {
  MockRule() {}

  MockRule(Index in_variables, Score in_bound) {
    variables = in_variables;
    bound = in_bound;
  }
  typedef FakeFinal Final;
  Index Variables() const { return variables; }
  Score Bound() const { return bound; }

  Index variables;
  Score bound;
};

FakeFinal::FakeFinal(const Context<FakeFinal> &, const MockRule &rule, std::vector<const FakeFinal*> &values) {
  BOOST_CHECK_EQUAL(rule.Variables(), values.size());
}

BOOST_AUTO_TEST_CASE(binary) {
  Context<FakeFinal> context;

  Edge<MockRule> base_edge;
  base_edge.InitRule() = MockRule(0, 5.78);
  base_edge.FinishedAdding(context);
  BOOST_REQUIRE_EQUAL(1, base_edge.Size());
  BOOST_CHECK_CLOSE(3.14, base_edge[0].Total(), 0.0001);

  Vertex<Edge<MockRule> > vertex;
  vertex.Add(base_edge);
  vertex.FinishedAdding();

  Edge<MockRule> edge;
  edge.InitRule() = MockRule(2, 13.2);
  edge.Add(vertex);
  edge.Add(vertex);
  edge.FinishedAdding(context);

  BOOST_CHECK_EQUAL(0, edge.Size());
  BOOST_CHECK_CLOSE(13.2 + 3.14 * 2, edge.Bound(), 0.0001);
  edge.More(context, edge.Bound());
  BOOST_CHECK_EQUAL(1, edge.Size());
  BOOST_CHECK_CLOSE(3.14, edge[0].Total(), 0.0001);
  BOOST_CHECK_EQUAL(-kScoreInf, edge.Bound());
}

}
} // namespace search
