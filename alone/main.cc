#include "alone/assemble.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "search/context.hh"
#include "util/file_piece.hh"
#include "util/usage.hh"

#include <iostream>

namespace alone {

void Decode(const char *lm_file, StringPiece weight_str) {
  Weights weights(weight_str);
  lm::ngram::RestProbingModel lm(lm_file);
  util::FilePiece graph_file(0, "stdin", &std::cerr);

  while (true) {
    search::Context context(lm, weights);
    Graph graph;
    if (!ReadCDec(context, graph_file, graph)) break;
    for (std::size_t i = 0; i < graph.VertexSize(); ++i) {
      search::VertexGenerator(context, graph.MutableVertex(i));
    }

    const search::PartialVertex &top = graph.Root().RootPartial();
    if (!top.Size()) {
      std::cout << "Empty" << std::endl;
    } else {

    }

    Graph::Vertex &root = graph.Root();
    float beat = root.Bound();
    while (!root.Size() && (root.Bound() != -search::kScoreInf)) {
      std::cerr << root.Bound() << '\n';
      beat = root.Bound() - .01;
      root.More(context, beat);
    }
    if (root.Size() == 0) {
      std::cout << "Empty" << std::endl;
    } else {
      std::cout << root[0] << "||| " << root[0].Total() << std::endl;
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
