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

template <class Control> void ReadLoop(const std::string &graph_prefix, Control &control) {
  for (unsigned int sentence = 0; ; ++sentence) {
    std::stringstream name;
    name << graph_prefix << '/' << sentence;
    std::auto_ptr<util::FilePiece> file;
    try {
      file.reset(new util::FilePiece(name.str().c_str()));
    } catch (const util::ErrnoException &e) {
      if (e.Error() == ENOENT) return;
      throw;
    }
    control.Add(file.release());
  }
}

void Run(const char *graph_prefix, const char *lm_file, StringPiece weight_str, unsigned int pop_limit, unsigned int threads) {
  lm::ngram::RestProbingModel lm(lm_file);
  search::Config config(lm, weight_str, pop_limit);

  if (threads > 1) {
#ifdef WITH_THREADS
    Controller controller(config, threads, std::cout);
    ReadLoop(graph_prefix, controller);
#else
    UTIL_THROW(util::Exception, "Threading support not compiled in.");
#endif
  } else {
    InThread controller(config, std::cout);
    ReadLoop(graph_prefix, controller);
  }
}

} // namespace alone

int main(int argc, char *argv[]) {
  if (argc < 5 || argc > 6) {
    std::cerr << argv[0] << " graph_prefix lm \"weights\" pop [threads]" << std::endl;
    return 1;
  }

#ifdef WITH_THREADS
  unsigned thread_count = boost::thread::hardware_concurrency();
#else
  unsigned thread_count = 1;
#endif
  if (argc == 6) {
    thread_count = boost::lexical_cast<unsigned>(argv[5]);
    UTIL_THROW_IF(!thread_count, util::Exception, "Thread count 0");
  }
  UTIL_THROW_IF(!thread_count, util::Exception, "Boost doesn't know how many threads there are.  Pass it on the command line.");
  alone::Run(argv[1], argv[2], argv[3], boost::lexical_cast<unsigned int>(argv[4]), thread_count);

  util::PrintUsage(std::cerr);
  return 0;
}
