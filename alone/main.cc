#include "alone/graph.hh"
#include "alone/read.hh"
#include "alone/threading.hh"
#include "search/config.hh"
#include "search/context.hh"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/usage.hh"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <memory>

namespace alone {

template <class Control> void ReadLoop(util::FilePiece &in, const search::Config &config, Control &control) {
  for (unsigned int sentence = 0; ; ++sentence) {
    std::auto_ptr<search::Context> context(new search::Context(config));
    std::auto_ptr<Graph> graph(new Graph);
    if (!ReadCDec(*context, in, *graph)) break;
    control.Add(context.release(), graph.release());
  }
}

void Decode(const char *lm_file, StringPiece weight_str, unsigned int pop_limit, unsigned int threads) {
  lm::ngram::RestProbingModel lm(lm_file);
  search::Config config(lm, weight_str, pop_limit);
  util::FilePiece graph_file(0, "stdin");

  if (threads > 1) {
#ifdef WITH_THREADS
    Controller controller(threads, std::cout);
    ReadLoop(graph_file, config, controller);
#else
    UTIL_THROW(util::Exception, "Threading support not compiled in.");
#endif
  } else {
    InThread controller(std::cout);
    ReadLoop(graph_file, config, controller);
  }
}

} // namespace alone

int main(int argc, char *argv[]) {
  if (argc < 4 || argc > 5) {
    std::cerr << argv[0] << " lm \"weights\" pop [threads] <graph" << std::endl;
    return 1;
  }

#ifdef WITH_THREADS
  unsigned thread_count = boost::thread::hardware_concurrency();
#else
  unsigned thread_count = 1;
#endif
  if (argc == 5) {
    thread_count = boost::lexical_cast<unsigned>(argv[4]);
    UTIL_THROW_IF(!thread_count, util::Exception, "Thread count 0");
  }
  UTIL_THROW_IF(!thread_count, util::Exception, "Boost doesn't know how many threads there are.  Pass it on the command line.");
  alone::Decode(argv[1], argv[2], boost::lexical_cast<unsigned int>(argv[3]), thread_count);

  util::PrintUsage(std::cerr);
  return 0;
}
