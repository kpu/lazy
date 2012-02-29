#include "alone/read.hh"
#include "util/file_piece.hh"

#include <iostream>

int main() {
  util::FilePiece f(0, "stdin", &std::cerr);
  alone::JustVocab(f, std::cout);
}
