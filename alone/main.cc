#include "alone/context.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "util/file_piece.hh"

namespace alone {

void Decode(const char *lm, const char *graph_name, StringPiece weights) {
  Context context(lm, weights);
  Graph graph;
  {
    util::FilePiece graph_file(graph_name, &std::cerr);
    ReadCDec(context, graph_file, graph);
  }
  Graph::Vertex &root = graph.Root();
  while (root.Size() < 1 && root.Bound() != -search::kScoreInf) root.More(context, root.Bound());
  if (root.Size() == 0) {
    std::cout << "Empty" << std::endl;
  } else {
    std::cout << "Got a solution." << std::endl;
  }
}

} // namespace alone

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << argv[0] << " lm graph \"weights\"" << std::endl;
    return 1;
  }
  alone::Decode(argv[1], argv[2], argv[3]);
  return 0;
}
