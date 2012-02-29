#include "alone/read.hh"

#include "alone/context.hh"
#include "alone/graph.hh"
#include "alone/weights.hh"
#include "util/file_piece.hh"

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
  ret->FinishedAdding(context);
  return *ret;
}

} // namespace

void ReadCDec(Context &context, util::FilePiece &from, Graph &to) {
  // Eat sentence
  from.ReadLine();
  unsigned long int vertices = from.ReadULong();
  unsigned long int edges = from.ReadULong();
  UTIL_THROW_IF(vertices == 0, FormatException, "Vertex count is zero");
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
}

} // namespace alone
