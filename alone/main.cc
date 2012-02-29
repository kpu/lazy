#include "alone/assemble.hh"
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
  root.More(context, -search::kScoreInf);
  if (root.Size() == 0) {
    std::cout << "Empty" << std::endl;
  } else {
    std::cout << root[0] << "||| " << root[0].Total() << std::endl;
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
