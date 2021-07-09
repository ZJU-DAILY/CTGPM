#ifndef MATCH_DEPENDENCYGRAPH_H
#define MATCH_DEPENDENCYGRAPH_H

#include <set>
#include <queue>
#include <cassert>
#include "SubGraph.h"
#include "DataGraph.h"
#include "DisjointSetUnion.h"

struct DependencyGraph {
    const DataGraph &data_graph;

    std::vector<SubGraph> sub_graphs;

    explicit DependencyGraph(const DataGraph &_dg) : data_graph(_dg) {}

    // use time-respecting rule to decompose graph
    void decompose() {
        const auto &data_nodes = data_graph.graph_data;

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
        std::vector<std::vector<std::pair<Pos, int>>> arr_dec_nodes(data_nodes.size()); // decompose node's pos && min_arrive_time
        std::vector<std::vector<std::pair<Pos, int>>> sta_dec_nodes(data_nodes.size()); // decompose node's pos && max_start_time
        for (const auto &ens : edge_n_stime) {
            auto [i, j] = ens.first;
            if (vis_data_edge.count({i, j})) continue;
            int sub_graph_id = sub_graphs.size();
            auto bfs = [&](int root_seq) {
                sub_graphs.emplace_back();
                auto &sub_graph = sub_graphs.back().graph_data;
                std::map<int, int> new_mapping;
                std::map<int, int> min_arrive_time, max_start_time;
                std::map<int, std::pair<int, std::map<int, int>>> in_edge_info; // seq -> maxmin, in_edge{seq, min-etime}
                std::map<int, std::pair<int, std::map<int, int>>> out_edge_info; // seq -> minmax, out_edge{seq, max-stime}
                new_mapping[root_seq] = sub_graph.size();
                std::queue<int> queue;
                DataNode root(root_seq, data_nodes[root_seq].identity, data_nodes[root_seq].label, {});
                for (int k = 0; k < (int)data_nodes[root_seq].adj_edge.size(); ++k) {
                    if (vis_data_edge.count({root_seq, k})) continue;
                    const auto &tmp_edge = data_nodes[root_seq].adj_edge[k];
                    root.adj_edge.push_back(tmp_edge);
                    queue.push(tmp_edge.destination_seq);
                    if (!max_start_time.count(root_seq)) {
                        max_start_time[root_seq] = tmp_edge.start_time;
                    } else {
                        max_start_time[root_seq] = std::max(max_start_time[root_seq], tmp_edge.start_time);
                    }
                    if (!min_arrive_time.count(tmp_edge.destination_seq)) {
                        min_arrive_time[tmp_edge.destination_seq] = tmp_edge.end_time;
                    } else {
                        min_arrive_time[tmp_edge.destination_seq] = std::min(min_arrive_time[tmp_edge.destination_seq],
                                                                             tmp_edge.end_time);
                    }
                    if (!in_edge_info.count(tmp_edge.destination_seq)) {
                        in_edge_info[tmp_edge.destination_seq].first = tmp_edge.end_time;
                        in_edge_info[tmp_edge.destination_seq].second[root_seq] = tmp_edge.end_time;
                    } else {
                        auto &max_min_etime = in_edge_info[tmp_edge.destination_seq].first;
                        auto &in_edge_set = in_edge_info[tmp_edge.destination_seq].second;
                        if (!in_edge_set.count(root_seq)) {
                            in_edge_set[root_seq] = tmp_edge.end_time;
                        } else {
                            in_edge_set[root_seq] = std::min(in_edge_set[root_seq], tmp_edge.end_time);
                        }
                        max_min_etime = std::max(max_min_etime, in_edge_set[root_seq]);
                    }
                    if (!out_edge_info.count(root_seq)) {
                        out_edge_info[root_seq].first = tmp_edge.start_time;
                        out_edge_info[root_seq].second[tmp_edge.destination_seq] = tmp_edge.start_time;
                    } else {
                        auto &min_max_stime = out_edge_info[root_seq].first;
                        auto &out_edge_set = out_edge_info[root_seq].second;
                        if (!out_edge_set.count(tmp_edge.destination_seq)) {
                            out_edge_set[tmp_edge.destination_seq] = tmp_edge.start_time;
                        } else {
                            out_edge_set[tmp_edge.destination_seq] = std::max(out_edge_set[tmp_edge.destination_seq],
                                                                              tmp_edge.start_time);
                        }
                        min_max_stime = std::min(min_max_stime, out_edge_set[tmp_edge.destination_seq]);
                    }
                    vis_data_edge[{root_seq, k}] = true;
                }
                sub_graph.push_back(root);
                while (!queue.empty()) {
                    auto seq = queue.front();
                    queue.pop();
                    if (new_mapping.count(seq)) continue;
                    new_mapping[seq] = sub_graph.size();
                    DataNode new_node(seq, data_nodes[seq].identity, data_nodes[seq].label, {});
                    for (int k = 0; k < (int)data_nodes[seq].adj_edge.size(); ++k) {
                        if (vis_data_edge.count({seq, k})) continue;
                        const auto &tmp_edge = data_nodes[seq].adj_edge[k];
                        if (tmp_edge.start_time < in_edge_info[seq].first) continue;
                        if (out_edge_info.count(tmp_edge.destination_seq) &&
                            tmp_edge.end_time > out_edge_info[tmp_edge.destination_seq].first) continue;
                        new_node.adj_edge.push_back(tmp_edge);
                        queue.push(tmp_edge.destination_seq);
                        if (!max_start_time.count(seq)) {
                            max_start_time[seq] = tmp_edge.start_time;
                        } else {
                            max_start_time[seq] = std::max(max_start_time[seq], tmp_edge.start_time);
                        }
                        if (!min_arrive_time.count(tmp_edge.destination_seq)) {
                            min_arrive_time[tmp_edge.destination_seq] = tmp_edge.end_time;
                        } else {
                            min_arrive_time[tmp_edge.destination_seq] = std::min(min_arrive_time[tmp_edge.destination_seq],
                                                                                 tmp_edge.end_time);
                        }
                        if (!in_edge_info.count(tmp_edge.destination_seq)) {
                            in_edge_info[tmp_edge.destination_seq].first = tmp_edge.end_time;
                            in_edge_info[tmp_edge.destination_seq].second[seq] = tmp_edge.end_time;
                        } else {
                            auto &max_min_etime = in_edge_info[tmp_edge.destination_seq].first;
                            auto &in_edge_set = in_edge_info[tmp_edge.destination_seq].second;
                            if (!in_edge_set.count(seq)) {
                                in_edge_set[seq] = tmp_edge.end_time;
                            } else {
                                in_edge_set[seq] = std::min(in_edge_set[seq], tmp_edge.end_time);
                            }
                            max_min_etime = std::max(max_min_etime, in_edge_set[seq]);
                        }
                        if (!out_edge_info.count(seq)) {
                            out_edge_info[seq].first = tmp_edge.start_time;
                            out_edge_info[seq].second[tmp_edge.destination_seq] = tmp_edge.start_time;
                        } else {
                            auto &min_max_stime = out_edge_info[seq].first;
                            auto &out_edge_set = out_edge_info[seq].second;
                            if (!out_edge_set.count(tmp_edge.destination_seq)) {
                                out_edge_set[tmp_edge.destination_seq] = tmp_edge.start_time;
                            } else {
                                out_edge_set[tmp_edge.destination_seq] = std::max(out_edge_set[tmp_edge.destination_seq],
                                                                                  tmp_edge.start_time);
                            }
                            min_max_stime = std::min(min_max_stime, out_edge_set[tmp_edge.destination_seq]);
                        }
                        vis_data_edge[{seq, k}] = true;
                    }
                    sub_graph.push_back(new_node);
                }
                // remapping
                auto &label_seqs = sub_graphs.back().label_seqs;
                for (auto &node : sub_graph) {
                    int new_seq = new_mapping[node.seq];
                    if (min_arrive_time.count(node.seq))
                        arr_dec_nodes[node.seq].push_back({{sub_graph_id, new_seq}, min_arrive_time[node.seq]});
                    if (max_start_time.count(node.seq))
                        sta_dec_nodes[node.seq].push_back({{sub_graph_id, new_seq}, max_start_time[node.seq]});
                    node.seq = new_seq;
                    if (label_seqs.count(node.label))
                        label_seqs[node.label].push_back(node.seq);
                    else
                        label_seqs[node.label] = {node.seq};
                    for (auto &edge : node.adj_edge) {
                        edge.source_seq = new_seq;
                        edge.destination_seq = new_mapping[edge.destination_seq];
                    }
                }
            };
            bfs(i);
        }
        // build relation
        DisjointSetUnion f(sub_graphs.size());
        for (int i = 0; i < (int)data_nodes.size(); ++i) {
            std::sort(sta_dec_nodes[i].begin(), sta_dec_nodes[i].end(), [](const auto &lhs, const auto &rhs) {
                return lhs.second > rhs.second;
            });
            for (const auto &arr_dec_node : arr_dec_nodes[i]) {
                for (const auto &sta_dec_node : sta_dec_nodes[i]) {
                    if (arr_dec_node.second > sta_dec_node.second) break;
                    if (arr_dec_node.first == sta_dec_node.first) continue;
                    sub_graphs[arr_dec_node.first.first].topo_order_relations.emplace_back(arr_dec_node.first.second, sta_dec_node.first);
                    int rl = f.find(arr_dec_node.first.first), rh = f.find(sta_dec_node.first.first);
                    if (rl != rh) f.fa[rl] = rh;
                }
            }
        }
        auto time_st = std::chrono::system_clock::now();
        // build_encoding && build_dist
        for (auto &sg : sub_graphs) {
            sg.build_encoding();
            sg.build_dist();
            std::sort(sg.topo_order_relations.begin(), sg.topo_order_relations.end(),
                      [&](const auto &lhs, const auto &rhs) {
                          return sg.topo_layer[lhs.first].second < sg.topo_layer[rhs.first].second;
                      });
            sg.adj_relations.resize(sg.graph_data.size());
            for (const auto &e : sg.topo_order_relations) {
                sg.adj_relations[e.first].push_back(e.second);
            }
        }
        auto time_en = std::chrono::system_clock::now();
        // std::cout << "build dep-part cost :" << std::chrono::duration_cast<std::chrono::milliseconds>(time_en - time_st).count() << '\n';
    }
};

#endif //MATCH_DEPENDENCYGRAPH_H