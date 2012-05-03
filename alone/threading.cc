#include "alone/threading.hh"

#include "alone/assemble.hh"
#include "alone/graph.hh"

#include "search/context.hh"
#include "search/vertex_generator.hh"

#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>

#include <sstream>

namespace alone {

void DecodeHandler::operator()(Input message) {
  boost::scoped_ptr<search::Context> context(message.context);
  boost::scoped_ptr<Graph> graph(message.graph);
  for (std::size_t i = 0; i < graph->VertexSize(); ++i) {
    search::VertexGenerator(*context, graph->MutableVertex(i));
  }
  search::PartialVertex top = graph->Root().RootPartial();
  if (top.Empty()) {
    Produce(message.sentence_id, "Empty");
  } else {
    search::PartialVertex continuation, ignored;
    while (!top.Complete()) {
      top.Split(continuation, ignored);
      top = continuation;
    }
    std::stringstream assemble;
    assemble << top.End() << " ||| " << top.End().Bound();
    Produce(message.sentence_id, assemble.str());
  }
}

void DecodeHandler::Produce(unsigned int sentence_id, const std::string &str) {
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
    out_ << *lead << '\n';
    delete lead;
  }
}

Controller::Controller(size_t decode_workers, std::ostream &to) : 
  sentence_id_(0),
  printer_(decode_workers, 1, boost::ref(to), Output::Poison()),
  decoder_(3, decode_workers, boost::ref(printer_.In()), Input::Poison()) {}

} // namespace alone
