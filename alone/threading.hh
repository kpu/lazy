#ifndef ALONE_THREADING__
#define ALONE_THREADING__

#include "util/pcqueue.hh"
#include "util/pool.hh"

#include <sstream>
#include <queue>
#include <string>

namespace search {
class Config;
class Context;
} // namespace search

namespace alone {

class Graph;

void Decode(search::Context *context, Graph *graph, std::ostream &out);

struct SentenceID {
  unsigned int sentence_id;
  bool operator==(const SentenceID &other) const {
    return sentence_id == other.sentence_id;
  }
};

struct Input : public SentenceID {
  search::Context *context;
  Graph *graph;
  static Input Poison() {
    Input ret;
    ret.sentence_id = static_cast<unsigned int>(-1);
    ret.context = NULL;
    ret.graph = NULL;
    return ret;
  }
};

struct Output : public SentenceID {
  std::string *str;
  static Output Poison() {
    Output ret;
    ret.sentence_id = static_cast<unsigned int>(-1);
    ret.str = NULL;
    return ret;
  }
};

class DecodeHandler {
  public:
    typedef Input Request;

    explicit DecodeHandler(util::PCQueue<Output> &out) : out_(out) {}

    void operator()(Input message);

  private:
    void Produce(unsigned int sentence_id, const std::string &str);
    
    util::PCQueue<Output> &out_;
};

class PrintHandler {
  public:
    typedef Output Request;

    explicit PrintHandler(std::ostream &o) : out_(o), done_(0) {}

    void operator()(Output message);

  private:
    std::ostream &out_;
    std::deque<std::string*> waiting_;
    unsigned int done_;
};

class Controller {
  public:
    // This config must remain valid.   
    explicit Controller(size_t decode_workers, std::ostream &to);

    // Takes ownership of graph.  
    void Add(search::Context *context, Graph *graph) {
      Input input;
      input.sentence_id = sentence_id_++;
      input.context = context;
      input.graph = graph;
      decoder_.Produce(input);
    }

  private:
    unsigned int sentence_id_;

    util::Pool<PrintHandler> printer_;

    util::Pool<DecodeHandler> decoder_;
};

} // namespace alone
#endif // ALONE_THREADING__
