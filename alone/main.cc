#include "alone/threading.hh"

#include "lm/model.hh"
#include "search/config.hh"
#include "search/context.hh"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/string_piece.hh"
#include "util/usage.hh"

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <memory>

#include <errno.h>

namespace alone {

template <class Control> void ReadLoop(StringPiece graph_dir, Control &control) {
  for (unsigned int sentence = 0; ; ++sentence) {
    std::stringstream name;
    name << graph_dir << '/' << sentence;
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

template <class Model> void RunWithModelType(StringPiece graph_dir, const std::string &lm_name, const search::Config &config, const feature::Weights &weights, unsigned int threads) {
  Model model(lm_name.c_str());

  if (threads > 1) {
#ifdef WITH_THREADS
    Controller<Model> controller(config, model, weights, threads, std::cout);
    ReadLoop(graph_dir, controller);
#else
    UTIL_THROW(util::Exception, "Threading support not compiled in.");
#endif
  } else {
    InThread<Model> controller(config, model, weights, std::cout);
    ReadLoop(graph_dir, controller);
  }
}

void Run(StringPiece graph_dir, const std::string &lm_name, const search::Config &config, const feature::Weights &weights, unsigned int threads) {
  lm::ngram::ModelType model_type;
  if (!lm::ngram::RecognizeBinary(lm_name.c_str(), model_type)) model_type = lm::ngram::PROBING;
  switch (model_type) {
    case lm::ngram::PROBING:
      RunWithModelType<lm::ngram::ProbingModel>(graph_dir, lm_name, config, weights, threads);
      break;
    case lm::ngram::REST_PROBING:
      RunWithModelType<lm::ngram::RestProbingModel>(graph_dir, lm_name, config, weights, threads);
      break;
    case lm::ngram::TRIE:
      RunWithModelType<lm::ngram::TrieModel>(graph_dir, lm_name, config, weights, threads);
      break;
    case lm::ngram::QUANT_TRIE:
      RunWithModelType<lm::ngram::QuantTrieModel>(graph_dir, lm_name, config, weights, threads);
      break;
    case lm::ngram::ARRAY_TRIE:
      RunWithModelType<lm::ngram::ArrayTrieModel>(graph_dir, lm_name, config, weights, threads);
      break;
    case lm::ngram::QUANT_ARRAY_TRIE:
      RunWithModelType<lm::ngram::QuantArrayTrieModel>(graph_dir, lm_name, config, weights, threads);
      break;
    default:
      UTIL_THROW(util::Exception, "Sorry this lm type isn't supported yet.");
  }
}

} // namespace alone

int main(int argc, char *argv[]) {
  try {
    namespace po = boost::program_options;
    po::options_description options("Decoding options");
    std::string graph_dir, lm_file, weights_file;
    unsigned beam, nbest;
    unsigned threads =
#ifdef WITH_THREADS
      boost::thread::hardware_concurrency();
#else
    1;
#endif
    options.add_options()
      ("graph_dir,i", po::value<std::string>(&graph_dir)->required(), "Directory in which input hypergraphs live.  The directory should contain one file per sentence, numbered consecutively starting with 0.")
      ("lm,l", po::value<std::string>(&lm_file)->required(), "Language model file to be loaded with KenLM.  Binary is preferred, but ARPA is also accepted.")
      ("weights,w", po::value<std::string>(&weights_file)->required(), "Weights file.  Format is whitespace-delimited pairs of feature_name weight.  Put = or whitespace between the feature name and the weight.  The hard-coded features are LanguageModel, OOV, and WordPenalty.")
      ("beam,K", po::value<unsigned>(&beam)->required(), "Beam size aka pop limit")
      ("k_best,k", po::value<unsigned>(&nbest)->default_value(1), "k-best list size")
      ("threads,t", po::value<unsigned>(&threads)->default_value(threads), "Number of threads to use")
      ("help,h", "Show help message");

    if (argc == 1) {
      std::cerr << options << std::endl;
      return 1;
    }

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cerr << options << std::endl;
      return 1;
    }
    UTIL_THROW_IF(!threads, util::Exception, "Thread count 0");

    using namespace alone;
    feature::Weights weights(feature::Weights::FromFile(), weights_file.c_str());
    search::Config config(weights.Lookup(feature::Computer::kLanguageModelName), beam, search::NBestConfig(nbest));
    Run(graph_dir, lm_file, config, weights, threads);

    util::PrintUsage(std::cerr);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
