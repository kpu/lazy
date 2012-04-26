#include "alone/assemble.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "search/context.hh"
#include "search/vertex_generator.hh"
#include "search/weights.hh"
#include "util/file_piece.hh"
#include "util/usage.hh"

#include <iostream>

namespace alone {

void Decode(const char *lm_file, StringPiece weight_str) {
  search::Weights weights(weight_str);
  lm::ngram::RestProbingModel lm(lm_file);
  util::FilePiece graph_file(0, "stdin", &std::cerr);

  while (true) {
    search::Context context(lm, weights);
    Graph graph;
    if (!ReadCDec(context, graph_file, graph)) break;
    for (std::size_t i = 0; i < graph.VertexSize(); ++i) {
      std::cerr << "Vertex " << i << " of " << graph.VertexSize() << std::endl; 
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
  if (argc != 3) {
    std::cerr << argv[0] << " lm \"weights\" <graph" << std::endl;
    return 1;
  }
  alone::Decode(argv[1], argv[2]);
  return 0;
}
