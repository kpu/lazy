#include "search/vertex_generator.hh"

#include "search/context.hh"

#include "lm/left.hh"

namespace search {

VertexGenerator::VertexGenerator(Context &context, Vertex &gen) : context_(context), edges_(gen.edges_.size()) {
  for (std::size_t i = 0; i < gen.edges_.size(); ++i) {
    edges_[i].Init(*gen.edges_[i]);
    generate_.push(&edges_[i]);
  }
  root_.under = &gen.root_;
}

void VertexGenerator::NewHypothesis(const lm::ngram::ChartState &state, const Edge &from, const PartialEdge &partial) {

}

VertexGenerator::Trie &VertexGenerator::Trie::Advance(Word word) {
  extend.insert(
}

} // namespace search
