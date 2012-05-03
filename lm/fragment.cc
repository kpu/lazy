#include "lm/left.hh"
#include "lm/model.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Pass lm" << std::endl;
    return 1;
  }
  lm::ngram::RestProbingModel m(argv[1]);
  util::FilePiece f(0, "stdin");
  StringPiece str;
  while (true) {
    float test;
/*    try {
      test = f.ReadFloat();
    } catch (const util::EndOfFileException &e) { break; }*/
    str = f.ReadLine();
    lm::ngram::ChartState state;
    lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(m, state);
    for (util::TokenIter<util::SingleCharacter, true> i(str, ' '); i; ++i) {
      scorer.Terminal(m.GetVocabulary().Index(*i));
    }
    float actual = scorer.Finish();
    std::cout << actual << std::endl;
/*    if (fabs(test - actual) > 0.0001) {
      std::cout << actual << " != " << test << " for" << str << std::endl;
    }*/
  }
}
