#include "alone/context.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "util/file_piece.hh"

namespace alone {

void Decode(const char *lm, util::FilePiece &graph_file, StringPiece weights) {
  Context context(lm, weights);
  Graph graph;
  ReadCDec(context, graph_file, graph);
}

} // namespace alone

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << argv[0] << " lm graph \"weights\"" << std::endl;
    return 1;
  }
  util::FilePiece graph(argv[2], &std::cerr);
  alone::Decode(argv[1], graph, argv[3]);
  return 0;
}
