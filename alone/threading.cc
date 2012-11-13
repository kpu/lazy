#include "alone/threading.hh"

#include "alone/assemble.hh"
#include "alone/config.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "alone/vocab.hh"
#include "lm/model.hh"
#include "search/applied.hh"
#include "search/context.hh"
#include "search/edge_generator.hh"
#include "search/nbest.hh"
#include "search/vertex_generator.hh"

#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <sstream>

namespace alone {

// Return root's history.  
template <class Model, class Best> search::History InnerDecode(Config &config, search::Context<Model> &context, Graph &graph, util::FilePiece &in, Best &best) {
  assert(graph.VertexSize());
  for (std::size_t v = 0; v < graph.VertexSize() - 1; ++v) {
    search::EdgeGenerator edges;
    ReadEdges(config, context, in, graph, edges);
    search::VertexGenerator<Best> vertex_gen(context, *graph.NewVertex(), best);
    edges.Search(context, vertex_gen);
  }
  search::EdgeGenerator edges;
  ReadEdges(config, context, in, graph, edges);
  search::Vertex &root = *graph.NewVertex();
  search::RootVertexGenerator<Best> vertex_gen(root, best);
  edges.Search(context, vertex_gen);
  return root.BestChild();
}

template <class Model> void Decode(Config &config, const Model &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out) {
  boost::scoped_ptr<util::FilePiece> in(in_ptr);
  search::Context<Model> context(config.GetSearch(), model);
  Graph graph(model.GetVocabulary());
  ReadGraphCounts(*in, graph);
  if (!graph.VertexSize()) {
    out << sentence_id << "NO PATH FOUND" << '\n';
    return;
  }

  if (config.GetSearch().GetNBest().size == 1) {
    search::SingleBest single_best;
    search::Applied best(InnerDecode(config, context, graph, *in, single_best));
    out << sentence_id << " ||| ";
    SingleLine(out, best, config.GetWeights());
    out << '\n';
  } else {
    search::NBest n_best(config.GetSearch().GetNBest());
    const std::vector<search::Applied> &applied = n_best.Extract(InnerDecode(config, context, graph, *in, n_best));
    for (std::vector<search::Applied>::const_iterator i = applied.begin(); i != applied.end(); ++i) {
      out << sentence_id << " ||| ";
      SingleLine(out, *i, config.GetWeights());
      out << '\n';
    }
  }
}

template void Decode(Config &config, const lm::ngram::ProbingModel &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(Config &config, const lm::ngram::RestProbingModel &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(Config &config, const lm::ngram::TrieModel &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(Config &config, const lm::ngram::QuantTrieModel &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(Config &config, const lm::ngram::ArrayTrieModel &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(Config &config, const lm::ngram::QuantArrayTrieModel &model, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out);

#ifdef WITH_THREADS
template <class Model> void DecodeHandler<Model>::operator()(Input message) {
  try {
    std::stringstream assemble;
    Decode(config_, model_, message.sentence_id, message.file, assemble);
    Produce(message.sentence_id, assemble.str());
  } catch (util::Exception &e) {
    e << " in sentence " << message.sentence_id;
    throw;
  }
}

template <class Model> void DecodeHandler<Model>::Produce(unsigned int sentence_id, const std::string &str) {
  Output out;
  out.sentence_id = sentence_id;
  out.str = new std::string(str);
  out_.Produce(out);
}

void PrintHandler::operator()(Output message) {
  unsigned int relative = message.sentence_id - done_;
  if (waiting_.size() <= relative) waiting_.resize(relative + 1);
  waiting_[relative] = message.str;
  for (std::string *lead; !waiting_.empty() && (lead = waiting_[0]); waiting_.pop_front(), ++done_) {
    out_ << *lead;
    delete lead;
  }
}

template <class Model> Controller<Model>::Controller(Config &config, const Model &model, size_t decode_workers, std::ostream &to) : 
  sentence_id_(0),
  printer_(decode_workers, 1, boost::ref(to), Output::Poison()),
  decoder_(3, decode_workers, boost::in_place(boost::ref(config), boost::ref(model), boost::ref(printer_.In())), Input::Poison()) {}

template class Controller<lm::ngram::RestProbingModel>;
template class Controller<lm::ngram::ProbingModel>;
template class Controller<lm::ngram::TrieModel>;
template class Controller<lm::ngram::QuantTrieModel>;
template class Controller<lm::ngram::ArrayTrieModel>;
template class Controller<lm::ngram::QuantArrayTrieModel>;

#endif

} // namespace alone
