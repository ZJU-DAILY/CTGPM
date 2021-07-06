#ifndef MATCH_DEPENDENCYGRAPH_HPP
#define MATCH_DEPENDENCYGRAPH_HPP

#include <set>
#include "Basic.hpp"
#include "DataGraph.hpp"

struct DependencyGraph {
    const DataGraph &data_graph;
    std::vector<DataNode> graph_data;
    std::map<std::string, std::vector<int>> label_seqs;
    std::vector<std::vector<int>> relations;

    explicit DependencyGraph(const DataGraph &_dg) : data_graph(_dg) {}

    void decompose() {
        const auto &data_nodes = data_graph.graph_data;

        using TimeStamp = std::pair<int, int>;
        std::vector<std::vector<TimeStamp>> node_time_stamps(data_nodes.size());
        for (const auto &node : data_nodes) {
            for (const auto &edge : node.adj_edge) {
                node_time_stamps[edge.source_seq].emplace_back(edge.start_time, 1);
                node_time_stamps[edge.destination_seq].emplace_back(edge.end_time, 0);
            }
        }
        std::vector<std::vector<int>> sub_nodes(data_nodes.size());
        std::map<std::pair<int, int>, int> mapping;
        for (int node_id = 0; node_id < node_time_stamps.size(); ++node_id) {
            auto &time_stamps = node_time_stamps[node_id];
            std::sort(time_stamps.begin(), time_stamps.end());
            int sz = time_stamps.size();
            for (int i = 0, j = 0; i < sz; i = j) {
                mapping[{node_id, sub_nodes[node_id].size()}] = graph_data.size();
                DataNode new_node;
                new_node.seq = graph_data.size();
                new_node.label = data_nodes[node_id].label;
                new_node.identity = data_nodes[node_id].identity;
                graph_data.push_back(new_node);
                sub_nodes[node_id].emplace_back(time_stamps[i].first);
                if (time_stamps[i].second == 0) {
                    while (j < sz && time_stamps[j].second == 0) ++j;
                }
                while (j < sz && time_stamps[j].second == 1) ++j;
            }
        }
        for (const auto &node : data_nodes) {
            for (const auto &edge : node.adj_edge) {
                int old_from = edge.source_seq;
                int new_from_pos = std::upper_bound(sub_nodes[old_from].begin(), sub_nodes[old_from].end(), edge.start_time) - sub_nodes[old_from].begin() - 1;
                int new_from = mapping[{old_from, new_from_pos}];
                int old_to = edge.destination_seq;
                int new_to_pos = std::upper_bound(sub_nodes[old_to].begin(), sub_nodes[old_to].end(), edge.end_time) - sub_nodes[old_to].begin() - 1;
                int new_to = mapping[{old_to, new_to_pos}];
                graph_data[new_from].adj_edge.emplace_back(new_from, new_to, edge.start_time, edge.end_time);
            }
        }
        if (false) {
            std::vector<std::pair<std::pair<int, int>, int>> edge_n_stime;
            for (int i = 0; i < (int)data_nodes.size(); ++i) {
                for (int j = 0; j < (int)data_nodes[i].adj_edge.size(); ++j) {
                    edge_n_stime.push_back({{i, j}, data_nodes[i].adj_edge[j].start_time});
                }
            }
            std::sort(edge_n_stime.begin(), edge_n_stime.end(), [](const auto &lhs, const auto &rhs) {
                return lhs.second < rhs.second;
            });
            std::map<std::pair<int, int>, bool> vis_data_edge;
            using Pos = std::pair<int, int>;
            int sub = 0;
            int cnt_num = 0;
            std::map<std::string, std::set<std::pair<int, int>>> st;
            for (const auto &ens : edge_n_stime) {
                auto &[i, j] = ens.first;
                if (vis_data_edge.count({i, j})) continue;
                sub++;
                auto bfs = [&](int i, int j) {
                    // std::set<int> nodes;
                    std::map<std::string, std::pair<int, std::pair<int, int>>> id_vis;
                    std::queue<int> queue;
                    id_vis[graph_data[i].identity] = {i, {std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}};
                    queue.push(i);
                    while (!queue.empty()) {
                        auto ii = queue.front();
                        queue.pop();
                        for (int kk = 0; kk < graph_data[ii].adj_edge.size(); ++kk) {
                            if (vis_data_edge[{ii, kk}]) continue;
                            auto &edge = graph_data[ii].adj_edge[kk];
                            if (id_vis.count(graph_data[edge.destination_seq].identity) && id_vis[graph_data[edge.destination_seq].identity].first != edge.destination_seq) continue;
                            if (id_vis[graph_data[ii].identity].second.first == std::numeric_limits<int>::min())
                                id_vis[graph_data[ii].identity].second.first = edge.start_time;
                            else
                                id_vis[graph_data[ii].identity].second.first = std::max(id_vis[graph_data[ii].identity].second.first, edge.start_time);
                            if (!id_vis.count(graph_data[edge.destination_seq].identity))
                                id_vis[graph_data[edge.destination_seq].identity] = {edge.destination_seq, {std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}};
                            if (id_vis[graph_data[edge.destination_seq].identity].second.second == std::numeric_limits<int>::max())
                                id_vis[graph_data[edge.destination_seq].identity].second.second = edge.end_time;
                            else
                                id_vis[graph_data[edge.destination_seq].identity].second.second = std::min(id_vis[graph_data[edge.destination_seq].identity].second.second, edge.end_time);
                            vis_data_edge[{ii, kk}] = true;
                            queue.push(edge.destination_seq);
                        }
                    }
                    for (const auto &e : id_vis) {
                        st[e.first].insert(e.second.second);
                    }
                    // std::cout << id_vis.size() << '\n';
                    cnt_num += id_vis.size();
                };
                bfs(i, j);
            }
            int reaa = 0;
            for (const auto e : st) {
                std::vector<std::pair<int, int>> tmp(e.second.begin(), e.second.end());
                for (int i = 0; i < tmp.size(); ++i) {
                    for (int j = i + 1; j < tmp.size(); ++j) {
                        if (tmp[i].second <= tmp[j].first) reaa++;
                        if (tmp[j].second <= tmp[i].first) reaa++;
                    }
                }
            }
            std::cout << sub << ' ' << cnt_num << ' ' << reaa << '\n';
        }
//        int reaaaa = 0;
//        relations.resize(graph_data.size());
//        for (int node_id = 0; node_id < sub_nodes.size(); ++node_id) {
//            int sz = sub_nodes[node_id].size();
//            for (int i = 0; i < sz; ++i) {
//                int map_id = mapping[{node_id, i}];
//                for (int j = i + 1; j < sz; ++j) {
//                    relations[map_id].push_back(mapping[{node_id, j}]);
//                    reaaaa++;
//                }
//            }
//        }
//        std::cout << graph_data.size() << " " << reaaaa << '\n';
        for (int node_id = 0; node_id < graph_data.size(); ++node_id) {
            const auto &node = graph_data[node_id];
            if (label_seqs.count(node.label)) {
                label_seqs[node.label].push_back(node_id);
            } else {
                label_seqs[node.label] = {node_id};
            }
        }
        // print_detail();
    }

    void print_detail() {
        for (int node_id = 0; node_id < graph_data.size(); ++node_id) {
            const auto &node = graph_data[node_id];
            std::cout << "node: " << node.seq << ' ' << node.label << ' ' << node.identity << '\n';
            for (const auto &edge : node.adj_edge) {
                std::cout << "    edge: " << edge.source_seq << ' ' << edge.destination_seq << ' ' << edge.start_time << ' ' << edge.end_time << '\n';
            }
            for (const auto &rea : relations[node_id]) {
                std::cout << "    relation: " << rea << '\n';
            }
        }
    }
};


#endif //MATCH_DEPENDENCYGRAPH_HPP
