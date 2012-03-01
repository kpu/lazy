#include "alone/assemble.hh"
#include "alone/context.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "util/file_piece.hh"
#include "util/usage.hh"

#include <iostream>

namespace alone {

void Decode(const char *lm_file, StringPiece weight_str) {
  Weights weights(weight_str);
  lm::ngram::RestProbingModel lm(lm_file);
  util::FilePiece graph_file(0, "stdin", &std::cerr);

  while (true) {
    Context context(lm, weights);
    Graph graph;
    try {
      ReadCDec(context, graph_file, graph);
    // TODO: mid-section EOF is still bad
    } catch (const util::EndOfFileException &e) { break; }
    Graph::Vertex &root = graph.Root();
    root.More(context, -search::kScoreInf);
    if (root.Size() == 0) {
      std::cout << "Empty" << std::endl;
    } else {
      std::cout << root[0] << "||| " << root[0].Total() << std::endl;
    }
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
