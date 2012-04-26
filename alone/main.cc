#include "alone/assemble.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "search/context.hh"
#include "search/vertex_generator.hh"
#include "search/weights.hh"
#include "util/ersatz_progress.hh"
#include "util/file_piece.hh"
#include "util/usage.hh"

#include <boost/lexical_cast.hpp>

#include <iostream>

namespace alone {

void Decode(const char *lm_file, StringPiece weight_str, unsigned int pop_limit) {
  search::Weights weights(weight_str);
  lm::ngram::RestProbingModel lm(lm_file);
  util::FilePiece graph_file(0, "stdin");

  for (unsigned int sentence = 0; ; ++sentence) {
    search::Context context(lm, weights, pop_limit);
    Graph graph;
    if (!ReadCDec(context, graph_file, graph)) break;
    std::cerr << "Sentence " << sentence << " has " << graph.VertexSize() << " and " << graph.EdgeSize() << " edges.";
    util::ErsatzProgress progress(graph.VertexSize());
    for (std::size_t i = 0; i < graph.VertexSize(); ++i, ++progress) {
      search::VertexGenerator(context, graph.MutableVertex(i));
    }

    search::PartialVertex top = graph.Root().RootPartial();
    if (top.Empty()) {
      std::cout << "Empty" << std::endl;
    } else {
      search::PartialVertex continuation, ignored;
      while (!top.Complete()) {
        top.Split(continuation, ignored);
        top = continuation;
      }
      std::cout << top.End() << " ||| " << top.End().Bound() << std::endl;
      DetailedFinal(std::cerr, top.End());
    }
    util::PrintUsage(std::cerr);
  }
}

} // namespace alone

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << argv[0] << " lm \"weights\" pop <graph" << std::endl;
    return 1;
  }
  alone::Decode(argv[1], argv[2], boost::lexical_cast<unsigned int>(argv[3]));
  return 0;
}
