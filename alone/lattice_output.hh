#ifndef SEARCH_LATTICE_OUTPUT__
#define SEARCH_LATTICE_OUTPUT__

#include "search/edge.hh"

#include <cstddef>
#include <vector>

namespace alone {

class Edge;

class LatticeOutput {
  public:
    typedef std::vector<search::PartialEdge> Combine;

    LatticeOutput(const Edge *edge_base) : edge_base_(edge_base), counter_(0) {}

    void Add(std::vector<search::PartialEdge> &existing, search::PartialEdge addition) const {
      existing.push_back(addition);
    }

    search::NBestComplete Complete(std::vector<search::PartialEdge> &partials);

  private:
    const Edge *edge_base_;
    intptr_t counter_;
};

} // namespace alone

#endif // SEARCH_LATTICE_OUTPUT__
