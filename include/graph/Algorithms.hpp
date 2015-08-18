#ifndef GRAPH_ALGORITHMS_HPP
#define GRAPH_ALGORITHMS_HPP

#include <vector>
#include <stack>
#include <random>
#include <algorithm>
#include <graph/Graph.hpp>

namespace graph {
	/**
	 * Get list of edges as vertex pairs.
	 * \param g Graph
	 * \param out Vector to fill
	 */
	template<class G, class V>
	inline void get_edges(const G &g, std::vector<std::pair<V,V>> &out) {
		out.clear();
		for(V u = 0; u < g.vertexCount(); ++u) {
			for(auto it = g.getAdjacent(u); it.first != it.second; ++it.first) {
				V v = *it.first;
				if(u <= v) {
					out.push_back(std::pair<V,V>(u, v));
				}
			}
		}
	}

	/**
	 * Adds a list of vertex pairs 
	 * \param in List of vertex pairs
	 * \param g Graph
	 */
	template<class G, class V>
	inline void add_edges(const std::vector<std::pair<V,V>> &in, G &g) {
		for(auto &e : in) {
			g.addEdge(e.first, e.second);
		}
	}

	template<class G, class V>
	inline bool has_edge(const G &g, V u, V v) {
		for(auto it = g.getAdjacent(u); it.first != it.second; ++it.first) {
			if(*it.first == v) return true;
		}

		return false;
	}

	/**
	 * Creates subgraph from list of vertices.
	 */
	template<class G, class V>
	inline void subgraph(const G &g, const std::vector<V> &indices, G &out) {
		out = G(indices.size());

		for(size_t i = 0; i < indices.size(); ++i) {
			out.node(i) = g.node(indices[i]);

			for(auto it = boost::out_edges(i, g.graph()); it.first != it.second; ++it.first) {
				size_t j = target(*it.first, g.graph());
				if(i <= j) {
					out.addEdge(i, j, g.edge(*it.first));
				}
			}
		}
	}

	/**
	 * Finds connected components in graph.
	 * Components are ordered 0, 1, 2, ...
	 *
	 * \param g Graph
	 * \param comp Output buffer
	 * \return List mapping each vertex to component
	 */
	template<class G, class V>
	inline void connectedComponents(const G &g, std::vector<V> &comp) {
		comp.resize(g.vertexCount());
		std::stack<V> stack;

		std::vector<bool> marked(g.vertexCount(), false);

		size_t covered = 0;
		int cur_comp = 0;
		while(covered < g.vertexCount()) {
			for(size_t i = 0; i < g.vertexCount(); ++i) {
				if(marked[i] == false) {
					stack.push(i);
					break;
				}
			}

			while(stack.empty() == false) {
				V u = stack.top();
				stack.pop();

				if(marked[u] == false) {
					marked[u] = true;
					covered++;
					comp[u] = cur_comp;

					for(auto it = g.getAdjacent(u); it.first != it.second; ++it.first) {
						stack.push(*it.first);
					}
				}
			}

			cur_comp++;
		}
	}

	/**
	 * Creates new graph containing only vertices in connected 
	 * components with at least min_size vertices.
	 * 
	 * \param g Graph
	 * \param min_size Miniumum component size
	 * \param out Output Output graph
	 */
	template<class G>
	inline void filterComponents(const G &g, int min_size, G &out) {
		typedef typename G::vertex_descriptor V;

		std::vector<V> comp;
		connectedComponents(g, comp);

		std::vector<int> count(g.vertexCount(), 0);
		std::vector<V> keep;
		
		for(auto i : comp) {
			count[i]++;
		}

		for(size_t i = 0; i < g.vertexCount(); ++i) {
			if(count[comp[i]] >= min_size) {
				keep.push_back(i);
			}
		}

		subgraph(g, keep, out);
	}

	/**
	 * Get list of vertices contained in largest connected component.
	 */
	template<class G, class V>
	inline void largestComponentIndices(const G &g, std::vector<V> &indices) {
		std::vector<V> comp;
		connectedComponents(g, comp);

		std::vector<int> count(g.vertexCount(), 0);
		indices.clear();

		for(auto i : comp) {
			count[i]++;
		}

		size_t largest = 0;
		for(size_t i = 0; i < g.vertexCount(); ++i) {
			if(count[i] > count[largest]) {
				largest = i;
			}
		}

		for(size_t i = 0; i < g.vertexCount(); ++i) {
			if(comp[i] == largest) {
				indices.push_back(i);
			}
		}
	}

	/**
	 * Creates new graph with only vertices contained in largest
	 * connected component.
	 */
	template<class G>
	inline void largestComponent(const G &g, G &out) {
		typedef typename G::vertex_descriptor V;
		std::vector<V> indices;
		largestComponentIndices(g, indices);
		subgraph(g, indices, out);
	}

	/**
	 * Randomizes edges of graph creating new graph
	 * with similar edge degree distribution.
	 */
	template<class G>
	inline void randomizeEndpoints(G &g, int count) {
		typedef typename G::vertex_descriptor V;
		std::vector<std::pair<V,V>> edges;
		get_edges(g, edges);

		std::default_random_engine gen;
		std::uniform_int_distribution<size_t> dist(0, edges.size()-1);

		int swaps = 0;
		while(swaps < count) {
			size_t e1 = dist(gen);
			size_t e2 = dist(gen);

			if(e1 == e2) continue;

			size_t a1, a2, b1, b2;

			if(gen() % 2 == 0) {
				a1 = edges[e1].first;
				a2 = edges[e1].second;
			} else {
				a1 = edges[e1].second;
				a2 = edges[e1].first;
			}

			if(gen() % 2 == 0) {
				b1 = edges[e2].first;
				b2 = edges[e2].second;
			} else {
				b1 = edges[e2].second;
				b2 = edges[e2].first;
			}

			if(a1 == b1 || a1 == b2
			|| a2 == b1 || a2 == b2) continue;

			if(has_edge(g, a1, b2) || has_edge(g, b1, a2)) continue;

			g.removeEdge(a1, a2);
			g.removeEdge(b1, b2);

			g.addEdge(a1, b2);
			g.addEdge(b1, a2);

			edges[e1] = std::make_pair(a1, b2);
			edges[e2] = std::make_pair(b1, a2);

			swaps++;
		}
	}
}

#endif
