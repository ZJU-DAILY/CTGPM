#ifndef MATCH_MATCHGRAPH_HPP
#define MATCH_MATCHGRAPH_HPP

#include <map>
#include "Basic.hpp"

struct MatchGraph {
    std::vector<MatchNode> graph_data;
    double score;

    void remapping() {
        std::map<int, int> new_mapping;
        for (int i = 0; i < (int) graph_data.size(); ++i) {
            new_mapping[graph_data[i].seq] = i;
        }
        for (auto &node : graph_data) {
            node.seq = new_mapping[node.seq];
            for (auto &edge : node.adj_edge) {
                edge.source_seq = new_mapping[edge.source_seq];
                edge.destination_seq = new_mapping[edge.destination_seq];
            }
        }
    }

    void calculate_score() {
        score = 0.0;
        for (const auto &node : graph_data) {
            for (const auto &adj_edge : node.adj_edge) {
                score += 1.0 / (1.0 + std::exp(adj_edge.length - 1)) + 0.5;
            }
        }
    }

    void print_detail() {
        std::cout << "\nmatch graph score " << score << '\n';
        std::cout << "nodes\n";
        for (const auto &node : graph_data) {
            std::cout << node.identity << ' ' << node.label << '\n';
        }
        std::cout << "edge\n";
        for (const auto &node : graph_data) {
            for (const auto &edge : node.adj_edge) {
                std::cout << graph_data[edge.source_seq].identity << ' ';
                std::cout << graph_data[edge.destination_seq].identity << ' ';
                std::cout << edge.start_time << ' ' << edge.end_time << ' ' << edge.length << '\n';
            }
        }
    }
};

#endif //MATCH_MATCHGRAPH_HPP
