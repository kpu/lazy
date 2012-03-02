#include "alone/read.hh"

#include "alone/context.hh"
#include "alone/graph.hh"
#include "alone/weights.hh"
#include "search/arity.hh"
#include "util/file_piece.hh"

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <cstdlib>

namespace alone {

namespace {

Graph::Edge &ReadEdge(Context &context, util::FilePiece &from, Graph &to, bool final) {
  Graph::Edge *ret = to.NewEdge();
  Rule &rule = ret->InitRule();
  StringPiece got;
  while ("|||" != (got = from.ReadDelimited())) {
    if ('[' == *got.data() && ']' == got.data()[got.size() - 1]) {
      // non-terminal
      char *end_ptr;
      unsigned long int child = std::strtoul(got.data() + 1, &end_ptr, 10);
      UTIL_THROW_IF(end_ptr != got.data() + got.size() - 1, FormatException, "Bad non-terminal" << got);
      UTIL_THROW_IF(child >= to.VertexSize(), FormatException, "Reference to vertex " << child << " but we only have " << to.VertexSize() << " vertices.  Is the file in bottom-up format?");
      ret->Add(to.GetVertex(child));
      rule.AppendNonTerminal();
    } else {
      rule.AppendTerminal(context.MutableVocab().FindOrAdd(got));
    }
  }
  if (final) rule.AppendTerminal(context.GetVocab().EndSentence());
  rule.FinishedAdding(context, context.GetWeights().DotNoLM(from.ReadLine()), final);
  UTIL_THROW_IF(rule.Variables() > search::kMaxArity, util::Exception, "Edit search/arity.hh and increase " << search::kMaxArity << " to at least " << rule.Variables());
  ret->FinishedAdding(context);
  return *ret;
}

} // namespace

// TODO: refactor
void JustVocab(util::FilePiece &from, std::ostream &out) {
  boost::unordered_set<std::string> seen;
  unsigned long int vertices = from.ReadULong();
  from.ReadULong(); // edges
  UTIL_THROW_IF(vertices == 0, FormatException, "Vertex count is zero");
  UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected newline after counts");
  std::string temp;
  for (unsigned long int i = 0; i < vertices; ++i) {
    unsigned long int edge_count = from.ReadULong();
    UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected after edge count");
    for (unsigned long int e = 0; e < edge_count; ++e) {
      StringPiece got;
      while ("|||" != (got = from.ReadDelimited())) {
        if ('[' == *got.data() && ']' == got.data()[got.size() - 1]) continue;
        temp.assign(got.data(), got.size());
        if (seen.insert(temp).second) out << temp << ' ';
      }
      from.ReadLine(); // weights
    }
  }
  // Eat sentence
  from.ReadLine();
}

void ReadCDec(Context &context, util::FilePiece &from, Graph &to) {
  unsigned long int vertices = from.ReadULong();
  unsigned long int edges = from.ReadULong();
  UTIL_THROW_IF(vertices < 2, FormatException, "Vertex count is " << vertices);
  UTIL_THROW_IF(edges == 0, FormatException, "Edge count is " << edges);
  --vertices;
  --edges;
  UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected newline after counts");
  to.SetCounts(vertices, edges);
  Graph::Vertex *vertex;
  for (unsigned long int i = 0; ; ++i) {
    vertex = to.NewVertex();
    unsigned long int edge_count = from.ReadULong();
    bool root = (i == vertices - 1);
    UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected after edge count");
    for (unsigned long int e = 0; e < edge_count; ++e) {
      vertex->Add(ReadEdge(context, from, to, root));
    }
    vertex->FinishedAdding();
    if (root) break;
  }
  to.SetRoot(vertex);
  StringPiece str = from.ReadLine();
  UTIL_THROW_IF("1" != str, FormatException, "Expected one edge to root");
  // The edge
  from.ReadLine();
  // The translation
  from.ReadLine();
}

} // namespace alone
