#ifndef MATCH_SUBGRAPH_H
#define MATCH_SUBGRAPH_H

#include "Basic.h"
#include <climits>

struct SubGraph {
    std::vector<DataNode> graph_data;
    std::vector<std::vector<int>> encoding;
    std::vector<std::pair<int, int>> topo_layer; // first and last
    std::vector<int> dists;
    std::vector<std::pair<int, std::pair<int, int>>> topo_order_relations; // topo-order
    std::vector<std::vector<std::pair<int, int>>> adj_relations;
    std::map<std::string, std::vector<int>> label_seqs;

    void build_encoding() {
        encoding.clear();
        topo_layer.resize(graph_data.size());
        std::fill(topo_layer.begin(), topo_layer.end(), std::make_pair(-1, -1));
        std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> edge_n_time;
        std::vector<int> in_degree(graph_data.size(), 0);
        for (int i = 0; i < (int)graph_data.size(); ++i) {
            for (int j = 0; j < (int)graph_data[i].adj_edge.size(); ++j) {
                edge_n_time.push_back({{i, j}, {graph_data[i].adj_edge[j].start_time, graph_data[i].adj_edge[j].end_time}});
                in_degree[graph_data[i].adj_edge[j].destination_seq]++;
            }
        }
        std::sort(edge_n_time.begin(), edge_n_time.end(), [](const auto &lhs, const auto &rhs) {
            if (lhs.second.first == rhs.second.first) return lhs.second.second < rhs.second.second;
            return lhs.second.first < rhs.second.first;
        });
        std::queue<int> queue;
        std::map<std::pair<int, int>, bool> vis_data_edge;
        for (int i = 0; i < (int)in_degree.size(); ++i) {
            if (in_degree[i] == 0) queue.push(i);
        }
        int num = 0, edge_ptr = 0;
        while (num < (int)graph_data.size()) {
            if (queue.empty()) {
                while (edge_ptr < (int)edge_n_time.size() && vis_data_edge.count(edge_n_time[edge_ptr].first)) ++edge_ptr;
                if (edge_ptr == (int)edge_n_time.size()) continue;
                vis_data_edge[edge_n_time[edge_ptr].first] = true;
                auto [i, j] = edge_n_time[edge_ptr].first;
                const auto &tmp_edge = graph_data[i].adj_edge[j];
                topo_layer[tmp_edge.source_seq] = {std::min(topo_layer[tmp_edge.source_seq].first, (int)encoding.size()),
                                                   std::max(topo_layer[tmp_edge.source_seq].second, (int)encoding.size())};
                encoding.push_back({tmp_edge.source_seq});
                in_degree[tmp_edge.destination_seq]--;
                if (in_degree[tmp_edge.destination_seq] == 0) queue.push(tmp_edge.destination_seq);
            } else {
                int sz = queue.size();
                std::vector<int> layer;
                for (int i = 0; i < sz; ++i, ++num) {
                    int seq = queue.front();
                    topo_layer[seq] = {std::min(topo_layer[seq].first, (int)encoding.size()),
                                       std::max(topo_layer[seq].second, (int)encoding.size())};
                    layer.push_back(seq);
                    queue.pop();
                    for (int j = 0; j < (int)graph_data[seq].adj_edge.size(); ++j) {
                        if (vis_data_edge.count({seq, j})) continue;
                        vis_data_edge[{seq, j}] = true;
                        in_degree[graph_data[seq].adj_edge[j].destination_seq]--;
                        if (in_degree[graph_data[seq].adj_edge[j].destination_seq] == 0)
                            queue.push(graph_data[seq].adj_edge[j].destination_seq);
                    }
                }
                encoding.push_back(std::move(layer));
            }
        }
    }

    void build_dist() {
        dists.resize(graph_data.size());
        std::fill(dists.begin(), dists.end(), std::numeric_limits<int>::max());
        std::vector<bool> vis(graph_data.size(), false);
        dists[0] = 0;
        vis[0] = true;
        std::queue<int> queue;
        queue.push(0);
        while (!queue.empty()) {
            int seq = queue.front();
            queue.pop();
            for (const auto &edge : graph_data[seq].adj_edge) {
                if (vis[edge.destination_seq]) continue;
                dists[edge.destination_seq] = dists[seq] + 1;
                vis[edge.destination_seq] = true;
                queue.push(edge.destination_seq);
            }
        }
    }

    void print_details() {
        std::cout << "graph's structure\n";
        for (const auto &sub_node : graph_data) {
            std::cout << "node: " << sub_node.seq << ' ' << sub_node.label << '\n';
//            for (const auto &edge : sub_node.adj_edge) {
//                std::cout << "edge: " << edge.destination_seq << ' ';
//                std::cout << graph_data[edge.destination_seq].label << ' ' << edge.start_time;
//                std::cout << ' ' << edge.end_time << '\n';
//            }
        }
//        std::cout << "sub-graph's layered-topo-order\n";
//        for (const auto &v : encoding) {
//            for (const auto &e : v) std::cout << e << ' ';
//            std::cout << '\n';
//        }
//        std::cout << "sub-graph's shortest distance\n";
//        for (auto e : dists) std::cout << e << ' ';
//        std::cout << '\n';
//        std::cout << "sub-graph's relations\n";
//        for(const auto &e : topo_order_relations) {
//            std::cout << e.first << " to (" << e.second.first << "," << e.second.second << ")\n";
//        }
    }
};

#endif //MATCH_SUBGRAPH_H
