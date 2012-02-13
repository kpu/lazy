#ifndef ALONE_GRAPH__
#define ALONE_GRAPH__

#include "alone/rule.hh"
#include "search/edge.hh"
#include "search/types.hh"
#include "search/vertex.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/scoped_array.hpp>

namespace alone {

class Graph {
  public:
    typedef search::Edge<Rule> Edge;
    typedef search::Vertex<Edge> Vertex;

    Graph() {}

    void SetCounts(std::size_t vertices, std::size_t edges) {
      vertices_.reset(new Vertex[vertices]);
      edges_.reset(new Edge[edges]);
    }

    Vertex *NewVertex() {
      return current_vertex_++;
    }

    std::size_t VertexSize() const { return current_vertex_ - vertices_.get(); }

    Vertex &GetVertex(search::Index index) {
      return vertices_[index];
    }


    Edge *NewEdge() {
      return current_edge_++;
    }

    void SetRoot(Vertex *root) {
      root_ = root;
    }

  private:
    boost::scoped_array<Vertex> vertices_;
    boost::scoped_array<Edge> edges_;

    Vertex *current_vertex_;
    Edge *current_edge_;

    Vertex *root_;
};

} // namespace alone

#endif // ALONE_GRAPH__
