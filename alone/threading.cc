#include "alone/threading.hh"

#include "alone/assemble.hh"
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
namespace {
// Return root's history.  
template <class Model, class Best> search::History InnerDecode(feature::Computer &features, search::Context<Model> &context, Graph &graph, util::FilePiece &in, Best &best) {
  assert(graph.VertexSize());
  for (std::size_t v = 0; v < graph.VertexSize() - 1; ++v) {
    search::EdgeGenerator edges;
    ReadEdges(features, context.LanguageModel(), in, graph, edges);
    search::VertexGenerator<Best> vertex_gen(context, *graph.NewVertex(), best);
    edges.Search(context, vertex_gen);
  }
  search::EdgeGenerator edges;
  ReadEdges(features, context.LanguageModel(), in, graph, edges);
  search::Vertex &root = *graph.NewVertex();
  search::RootVertexGenerator<Best> vertex_gen(root, best);
  edges.Search(context, vertex_gen);
  return root.BestChild();
}

template <class Model> void Decode(search::Context<Model> &context, feature::Computer &features, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out) {
  boost::scoped_ptr<util::FilePiece> in(in_ptr);
  Graph graph(context.LanguageModel().GetVocabulary());
  ReadGraphCounts(*in, graph);
  if (!graph.VertexSize()) {
    out << sentence_id << "NO PATH FOUND" << '\n';
    return;
  }

  if (context.GetConfig().GetNBest().size == 1) {
    search::SingleBest single_best;
    search::Applied best(InnerDecode(features, context, graph, *in, single_best));
    out << sentence_id << " ||| ";
    if (!best.Valid()) {
      out << "NO PATH FOUND" << '\n';
    } else {
      JustText(out, best);
      out << " ||| ";
      features.Write(out, best, context.LanguageModel());
      out << " ||| " << best.GetScore() << '\n';
    }
  } else {
    search::NBest n_best(context.GetConfig().GetNBest());
    const std::vector<search::Applied> &applied = n_best.Extract(InnerDecode(features, context, graph, *in, n_best));
    for (std::vector<search::Applied>::const_iterator i = applied.begin(); i != applied.end(); ++i) {
      out << sentence_id << " ||| ";
      JustText(out, *i);
      out << " ||| ";
      features.Write(out, *i, context.LanguageModel());
      out << " ||| " << i->GetScore() << '\n';
    }
  }
}
} // namespace

#ifdef WITH_THREADS
template <class Model> void DecodeHandler<Model>::operator()(Input message) {
  try {
    std::stringstream assemble;
    search::Context<Model> context(config_, model_);
    Decode(context, features_, message.sentence_id, message.file, assemble);
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

template <class Model> Controller<Model>::Controller(const search::Config &config, const Model &model, const feature::Weights &weights, size_t decode_workers, std::ostream &to) : 
  sentence_id_(0),
  printer_(decode_workers, 1, boost::ref(to), Output::Poison()),
  decoder_(
      3, // Buffer of three FilePieces for threads to read.  
      decode_workers,
      boost::in_place(boost::ref(config), boost::ref(model), boost::ref(weights), boost::ref(printer_.In())),
      Input::Poison()) {}

template class Controller<lm::ngram::RestProbingModel>;
template class Controller<lm::ngram::ProbingModel>;
template class Controller<lm::ngram::TrieModel>;
template class Controller<lm::ngram::QuantTrieModel>;
template class Controller<lm::ngram::ArrayTrieModel>;
template class Controller<lm::ngram::QuantArrayTrieModel>;

#endif

template <class Model> void InThread<Model>::Add(util::FilePiece *in) {
  search::Context<Model> context(config_, model_);
  Decode(context, features_, sentence_id_++, in, to_);
}

template class InThread<lm::ngram::RestProbingModel>;
template class InThread<lm::ngram::ProbingModel>;
template class InThread<lm::ngram::TrieModel>;
template class InThread<lm::ngram::QuantTrieModel>;
template class InThread<lm::ngram::ArrayTrieModel>;
template class InThread<lm::ngram::QuantArrayTrieModel>;


} // namespace alone
