#include "alone/threading.hh"

#include "alone/assemble.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "alone/vocab.hh"
#include "lm/model.hh"
#include "search/context.hh"

#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <sstream>

namespace alone {
template <class Model> void Decode(const search::Config &config, const Model &model, util::FilePiece *in_ptr, std::ostream &out) {
  search::Context<Model> context(config, model);
  Graph graph;
  Vocab vocab(model.GetVocabulary());
  {
    boost::scoped_ptr<util::FilePiece> in(in_ptr);
    ReadCDec(context, *in, graph, vocab);
  }

  search::Final best(graph.Root().BestChild());
  if (!best.Valid()) {
    out << "NO PATH FOUND";
  } else {
    out << best << " ||| " << best.GetScore() << std::endl;
  }
}

template void Decode(const search::Config &config, const lm::ngram::ProbingModel &model, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(const search::Config &config, const lm::ngram::RestProbingModel &model, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(const search::Config &config, const lm::ngram::TrieModel &model, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(const search::Config &config, const lm::ngram::QuantTrieModel &model, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(const search::Config &config, const lm::ngram::ArrayTrieModel &model, util::FilePiece *in_ptr, std::ostream &out);
template void Decode(const search::Config &config, const lm::ngram::QuantArrayTrieModel &model, util::FilePiece *in_ptr, std::ostream &out);

#ifdef WITH_THREADS
template <class Model> void DecodeHandler<Model>::operator()(Input message) {
  std::stringstream assemble;
  Decode(config_, model_, message.file, assemble);
  Produce(message.sentence_id, assemble.str());
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

template <class Model> Controller<Model>::Controller(const search::Config &config, const Model &model, size_t decode_workers, std::ostream &to) : 
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
