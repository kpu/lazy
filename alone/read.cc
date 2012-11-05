#include "alone/read.hh"

#include "alone/graph.hh"
#include "alone/vocab.hh"
#include "lm/model.hh"
#include "search/context.hh"
#include "search/edge.hh"
#include "search/edge_generator.hh"
#include "search/weights.hh"
#include "util/file_piece.hh"

#include <boost/unordered_map.hpp>

#include <algorithm>
#include <cstdlib>

namespace alone {

namespace {

template <class Model> void ReadEdge(search::Context<Model> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &generator) {
  EdgeWords &strings = *graph.NewEdgeWords();

  StringPiece got;
  std::vector<lm::WordIndex> words;
  std::vector<search::PartialVertex> children;
  unsigned long int terminals = 0;
  bool bos = false;
  float below_score = 0.0;
  while ("|||" != (got = from.ReadDelimited())) {
    if ('[' == *got.data() && ']' == got.data()[got.size() - 1]) {
      // non-terminal
      char *end_ptr;
      unsigned long int child = std::strtoul(got.data() + 1, &end_ptr, 10);
      UTIL_THROW_IF(end_ptr != got.data() + got.size() - 1, FormatException, "Bad non-terminal" << got);
      UTIL_THROW_IF(child >= graph.VertexSize(), FormatException, "Reference to vertex " << child << " but we only have " << graph.VertexSize() << " vertices.  Is the file in bottom-up format?");
      strings.AppendWord(NULL);
      words.push_back(lm::kMaxWordIndex);
      children.push_back(graph.GetVertex(child).RootPartial());
      if (children.back().Empty()) {
        from.ReadLine();
        return;
      }
      below_score += children.back().Bound();
    } else if (got == "<s>") {
      bos = true;
      ++terminals;
    } else {
      const std::pair<const std::string, lm::WordIndex> &found = graph.MutableVocab().FindOrAdd(got);
      strings.AppendWord(&found.first);
      words.push_back(found.second);
      ++terminals;
    }
  }

  search::PartialEdge edge(generator.AllocateEdge(children.size()));
  std::copy(children.begin(), children.end(), edge.NT());

  edge.SetScore(
    // Hypotheses below
    below_score +
    // Hard-coded word penalty.  
    context.GetWeights().DotNoLM(from.ReadLine()) - context.GetWeights().WordPenalty() * static_cast<float>(terminals) / M_LN10 +
    // Language model feature. 
    search::ScoreRule(context, words, bos, edge.Between()));

  search::Note note;
  note.vp = &strings;
  edge.SetNote(note);
  generator.AddEdge(edge);
}

} // namespace

void ReadGraphCounts(util::FilePiece &from, Graph &graph) {
  unsigned long int vertices = from.ReadULong();
  unsigned long int edges = from.ReadULong();
  UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected newline after counts");
  graph.SetCounts(vertices, edges);
}

template <class Model> void ReadEdges(search::Context<Model> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &edges) {
  unsigned long int edge_count = from.ReadULong();
  UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected newline after edge count");
  for (unsigned long int e = 0; e < edge_count; ++e) {
    ReadEdge(context, from, graph, edges);
  }
}

template void ReadEdges(search::Context<lm::ngram::ProbingModel> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(search::Context<lm::ngram::RestProbingModel> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(search::Context<lm::ngram::TrieModel> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(search::Context<lm::ngram::QuantTrieModel> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(search::Context<lm::ngram::ArrayTrieModel> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(search::Context<lm::ngram::QuantArrayTrieModel> &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);

} // namespace alone
