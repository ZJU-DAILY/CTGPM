/*
 * Copyright (c) 2021 by Contributors
 * \file DependencyGraph.hpp
 * \date 2021-10
 * \author Xinwei Cai
 */
#pragma once

#include "SubGraph.hpp"
#include "DataGraph.hpp"
#include "DisjointSetUnion.hpp"

struct DependencyGraph {
    DataGraph &data_graph;

    std::vector<SubGraph> sub_graphs;

    std::map<int, std::pair<int, int>> edge_belong_sub;
    std::map<std::pair<int, int>, int> edge_belong_data;

    std::vector<std::vector<std::pair<int, int>>> dec_nodes;

    explicit DependencyGraph(DataGraph &_dg) : data_graph(_dg) {}

    // use time-respecting rule to decompose graph
    void decompose() {
        const auto &data_nodes = data_graph.graph_data;
        const auto &data_edges = data_graph.edges;

        std::vector<int> sorted_edges(data_edges.size());
        std::iota(sorted_edges.begin(), sorted_edges.end(), 0);
        std::sort(sorted_edges.begin(), sorted_edges.end(), [&](const auto &lhs, const auto &rhs) {
            return data_edges[lhs].start_time < data_edges[rhs].start_time;
        });

        std::vector<bool> vis_data_edge(data_edges.size(), false);
        using Pos = std::pair<int, int>;
        dec_nodes.resize(data_nodes.size());

        std::map<int, int> new_mapping;
        std::map<int, std::pair<int, std::map<int, int>>> in_edge_info; // seq -> maxmin, in_edge{seq, min-etime}
        std::map<int, std::pair<int, std::map<int, int>>> out_edge_info; // seq -> minmax, out_edge{seq, max-stime}

        for (const auto &edge_id : sorted_edges) {
            if (vis_data_edge[edge_id]) continue;
            int i = data_edges[edge_id].source_seq;
            int sub_graph_id = (int) sub_graphs.size();
            auto bfs = [&](int root_seq) {
                new_mapping.clear();
                in_edge_info.clear();
                out_edge_info.clear();
                sub_graphs.emplace_back();
                auto &sub_graph = sub_graphs.back();
                auto &sub_edges = sub_graph.edges;
                auto &sub_nodes = sub_graph.graph_data;
                new_mapping[root_seq] = (int) sub_nodes.size();
                std::queue<int> queue;
                DataNode root(root_seq, data_nodes[root_seq].identity, data_nodes[root_seq].label, {});
                for (auto k : data_nodes[root_seq].adj_edge) {
                    if (vis_data_edge[k]) continue;
                    auto &tmp_edge = data_edges[k];
                    int new_edge_id = (int) sub_edges.size();
                    edge_belong_sub[k] = {sub_graph_id, new_edge_id};
                    edge_belong_data[{sub_graph_id, new_edge_id}] = k;
                    sub_edges.push_back(tmp_edge);
                    sub_edges.back().seq = new_edge_id;
                    root.adj_edge.push_back(new_edge_id);
                    queue.push(tmp_edge.destination_seq);
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
                        max_min_etime = I32_MIN;
                        for (auto e : in_edge_set) {
                            max_min_etime = std::max(max_min_etime, e.second);
                        }
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
                        min_max_stime = I32_MAX;
                        for (auto &e : out_edge_set) {
                            min_max_stime = std::min(min_max_stime, e.second);
                        }
                    }
                    if (!out_edge_info.count(tmp_edge.destination_seq)) {
                        out_edge_info[tmp_edge.destination_seq] = {I32_MAX, {}};
                    }
                    if (!in_edge_info.count(tmp_edge.source_seq)) {
                        in_edge_info[tmp_edge.source_seq] = {I32_MIN, {}};
                    }
                    vis_data_edge[k] = true;
                }
                sub_nodes.push_back(root);
                while (!queue.empty()) {
                    auto seq = queue.front();
                    queue.pop();
                    if (new_mapping.count(seq)) continue;
                    new_mapping[seq] = (int) sub_nodes.size();
                    DataNode new_node(seq, data_nodes[seq].identity, data_nodes[seq].label, {});
                    for (auto k : data_nodes[seq].adj_edge) {
                        if (vis_data_edge[k]) continue;
                        auto &tmp_edge = data_edges[k];
                        if (tmp_edge.start_time < in_edge_info[seq].first) continue;
                        if (out_edge_info.count(tmp_edge.destination_seq) &&
                        tmp_edge.end_time > out_edge_info[tmp_edge.destination_seq].first) continue;
                        int new_edge_id = (int) sub_edges.size();
                        edge_belong_sub[k] = {sub_graph_id, new_edge_id};
                        edge_belong_data[{sub_graph_id, new_edge_id}] = k;
                        sub_edges.push_back(tmp_edge);
                        sub_edges.back().seq = new_edge_id;
                        new_node.adj_edge.push_back(new_edge_id);
                        queue.push(tmp_edge.destination_seq);
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
                            max_min_etime = I32_MIN;
                            for (auto e : in_edge_set) {
                                max_min_etime = std::max(max_min_etime, e.second);
                            }
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
                            min_max_stime = I32_MAX;
                            for (auto &e : out_edge_set) {
                                min_max_stime = std::min(min_max_stime, e.second);
                            }
                        }
                        if (!out_edge_info.count(tmp_edge.destination_seq)) {
                            out_edge_info[tmp_edge.destination_seq] = {I32_MAX, {}};
                        }
                        if (!in_edge_info.count(tmp_edge.source_seq)) {
                            in_edge_info[tmp_edge.source_seq] = {I32_MIN, {}};
                        }
                        vis_data_edge[k] = true;
                    }
                    sub_nodes.push_back(new_node);
                }
                // remapping
                for (auto &node : sub_nodes) {
                    int new_seq = new_mapping[node.seq];
                    dec_nodes[node.seq].emplace_back(sub_graph_id, new_seq);
                    node.seq = new_seq;
                    for (auto &edge_id : node.adj_edge) {
                        sub_edges[edge_id].source_seq = new_seq;
                        sub_edges[edge_id].destination_seq = new_mapping[sub_edges[edge_id].destination_seq];
                    }
                }
                sub_graph.node_seq_mapping = new_mapping;
                for (auto &e : sub_edges) {
                    sub_nodes[e.destination_seq].rev_edge.push_back(e.seq);
                }
                for (auto &e : in_edge_info) {
                    sub_graph.max_min_etime[new_mapping[e.first]] = e.second.first;
                }
                for (auto &e : out_edge_info) {
                    sub_graph.min_max_stime[new_mapping[e.first]] = e.second.first;
                }
                sub_graph.build_mtime();
                sub_graph.adj_relations.resize(sub_nodes.size());
            };
            bfs(i);
        }
        build_relation();
        // TODO : search space
        // build_encoding && build_dist
        /*for (auto &sg : sub_graphs) {
            sg.build_encoding();
            sg.build_dist();
            std::sort(sg.topo_order_relations.begin(), sg.topo_order_relations.end(),
                      [&](const auto &lhs, const auto &rhs) {
                return sg.topo_layer[lhs.first].second < sg.topo_layer[rhs.first].second;
            });
        }*/
        // std::cout << "build dep-part cost :" << std::chrono::duration_cast<std::chrono::milliseconds>(time_en - time_st).count() << '\n';
    }

    void build_relation() {
        for (auto &dec_node : dec_nodes) {
            for (int j = 0; j < dec_node.size(); ++j) {
                for (int k = 0; k < dec_node.size(); ++k) {
                    auto &lhs = dec_node[j], &rhs = dec_node[k];
                    if (lhs.first == rhs.first) continue;
                    if (sub_graphs[lhs.first].min_etime[lhs.second] <= sub_graphs[rhs.first].max_stime[rhs.second]) {
                        sub_graphs[lhs.first].adj_relations[lhs.second].push_back(rhs);
                    }
                }
            }
        }
    }

    std::set<int> lazy_update_r;
    void update_relation() {
        for (auto x : lazy_update_r) {
            update_single_relation(x);
        }
        lazy_update_r.clear();
    }

    void update_single_relation(int x) {
        for (int j = 0; j < dec_nodes[x].size(); ++j) {
            auto &lhs = dec_nodes[x][j];
            sub_graphs[lhs.first].adj_relations[lhs.second].clear();
            for (int k = 0; k < dec_nodes[x].size(); ++k) {
                auto &rhs = dec_nodes[x][k];
                if (lhs.first == rhs.first) continue;
                if (sub_graphs[lhs.first].min_etime[lhs.second] <= sub_graphs[rhs.first].max_stime[rhs.second]) {
                    sub_graphs[lhs.first].adj_relations[lhs.second].push_back(rhs);
                }
            }
        }
    }

    void insert_edge(std::string &line) {
        auto edge_id = data_graph.insert(line);
        while (data_graph.graph_data.size() > dec_nodes.size()) {
            dec_nodes.emplace_back();
        }
        insert_edge(edge_id);
    }

    /*void insert_edge(int edge_id) {
        const auto &new_edge = data_graph.edges[edge_id];
        int from = new_edge.source_seq, to = new_edge.destination_seq;
        int stime = new_edge.start_time, etime = new_edge.end_time;
        for (int sub_graph_id = 0; sub_graph_id < sub_graphs.size(); ++sub_graph_id) {
            auto &sub_graph = sub_graphs[sub_graph_id];
            auto &mp = sub_graph.node_seq_mapping;
            if (!mp.count(from) || !mp.count(to)) continue;
            int u = mp[from], v = mp[to];
            if (stime < sub_graph.max_min_etime[u] || etime > sub_graph.min_max_stime[v]) continue;
            int new_edge_id = (int) sub_graph.edges.size();
            edge_belong_sub[edge_id] = {sub_graph_id, new_edge_id};
            edge_belong_data[{sub_graph_id, new_edge_id}] = edge_id;
            sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
            sub_graph.graph_data[u].adj_edge.push_back(new_edge_id);
            sub_graph.graph_data[v].rev_edge.push_back(new_edge_id);
            sub_graph.update_stime(u);
            sub_graph.update_etime(v);
            update_relation(from);
            update_relation(to);
            return;
        }
        for (int sub_graph_id = 0; sub_graph_id < sub_graphs.size(); ++sub_graph_id) {
            auto &sub_graph = sub_graphs[sub_graph_id];
            auto &mp = sub_graph.node_seq_mapping;
            if (mp.count(from) && !mp.count(to)) {
                int u = mp[from];
                if (stime < sub_graph.max_min_etime[u]) continue;
                mp[to] = (int) sub_graph.graph_data.size();
                int v = mp[to];
                sub_graph.graph_data.push_back({v, data_graph.graph_data[to].identity, data_graph.graph_data[to].label, {}});
                sub_graph.adj_relations.emplace_back();
                int new_edge_id = (int) sub_graph.edges.size();
                edge_belong_sub[edge_id] = {sub_graph_id, new_edge_id};
                edge_belong_data[{sub_graph_id, new_edge_id}] = edge_id;
                sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
                sub_graph.graph_data[u].adj_edge.push_back(new_edge_id);
                sub_graph.graph_data[v].rev_edge.push_back(new_edge_id);
                sub_graph.update_stime(u);
                sub_graph.update_etime(v);
                update_relation(from);
                update_relation(to);
                return;
            } else if (!mp.count(from) && mp.count(to)) {
                int v = mp[to];
                if (etime > sub_graph.min_max_stime[v]) continue;
                mp[from] = (int) sub_graph.graph_data.size();
                int u = mp[from];
                sub_graph.graph_data.push_back({u, data_graph.graph_data[from].identity, data_graph.graph_data[from].label, {}});
                sub_graph.adj_relations.emplace_back();
                int new_edge_id = (int) sub_graph.edges.size();
                edge_belong_sub[edge_id] = {sub_graph_id, new_edge_id};
                edge_belong_data[{sub_graph_id, new_edge_id}] = edge_id;
                sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
                sub_graph.graph_data[u].adj_edge.push_back(new_edge_id);
                sub_graph.graph_data[v].rev_edge.push_back(new_edge_id);
                sub_graph.update_stime(u);
                sub_graph.update_etime(v);
                update_relation(from);
                update_relation(to);
                return;
            }
        }
        // new sub
        {
            sub_graphs.emplace_back();
            auto &sub_graph = sub_graphs.back();
            int u = 0, v = 1, new_edge_id = 0;
            sub_graph.node_seq_mapping[from] = u;
            sub_graph.node_seq_mapping[to] = v;
            sub_graph.graph_data.push_back({u, data_graph.graph_data[from].identity, data_graph.graph_data[from].label, {new_edge_id}});
            sub_graph.graph_data.push_back({v, data_graph.graph_data[to].identity, data_graph.graph_data[to].label, {new_edge_id}});
            sub_graph.adj_relations.emplace_back();
            sub_graph.adj_relations.emplace_back();
            sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
            sub_graph.update_stime(u);
            sub_graph.update_etime(v);
            lazy_update_r.insert(from);
            lazy_update_r.insert(to);
        }
    }*/

    void insert_edge(int edge_id) {
        const auto &new_edge = data_graph.edges[edge_id];
        int from = new_edge.source_seq, to = new_edge.destination_seq;
        int stime = new_edge.start_time, etime = new_edge.end_time;
        for (auto &[sub_graph_id, u] : dec_nodes[from]) {
            auto &sub_graph = sub_graphs[sub_graph_id];
            if (stime < sub_graph.max_min_etime[u]) continue;
            auto &mp = sub_graph.node_seq_mapping;
            if (mp.count(to)) {
                int v = mp[to];
                if (etime > sub_graph.min_max_stime[v]) continue;
                int new_edge_id = (int) sub_graph.edges.size();
                edge_belong_sub[edge_id] = {sub_graph_id, new_edge_id};
                edge_belong_data[{sub_graph_id, new_edge_id}] = edge_id;
                sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
                sub_graph.graph_data[u].adj_edge.push_back(new_edge_id);
                sub_graph.graph_data[v].rev_edge.push_back(new_edge_id);
                sub_graph.update_stime(u);
                sub_graph.update_etime(v);
                lazy_update_r.insert(from);
                lazy_update_r.insert(to);
                return;
            } else {
                mp[to] = (int) sub_graph.graph_data.size();
                int v = mp[to];
                dec_nodes[to].emplace_back(sub_graph_id, v);
                sub_graph.graph_data.push_back({v, data_graph.graph_data[to].identity, data_graph.graph_data[to].label, {}});
                sub_graph.adj_relations.emplace_back();
                int new_edge_id = (int) sub_graph.edges.size();
                edge_belong_sub[edge_id] = {sub_graph_id, new_edge_id};
                edge_belong_data[{sub_graph_id, new_edge_id}] = edge_id;
                sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
                sub_graph.graph_data[u].adj_edge.push_back(new_edge_id);
                sub_graph.graph_data[v].rev_edge.push_back(new_edge_id);
                sub_graph.update_stime(u);
                sub_graph.update_etime(v);
                lazy_update_r.insert(from);
                lazy_update_r.insert(to);
                return;
            }
        }
        for (auto &[sub_graph_id, v] : dec_nodes[to]) {
            auto &sub_graph = sub_graphs[sub_graph_id];
            if (stime < sub_graph.max_min_etime[v]) continue;
            auto &mp = sub_graph.node_seq_mapping;
            if (!mp.count(from)) {
                mp[from] = (int) sub_graph.graph_data.size();
                int u = mp[from];
                dec_nodes[from].emplace_back(sub_graph_id, u);
                sub_graph.graph_data.push_back({u, data_graph.graph_data[from].identity, data_graph.graph_data[from].label, {}});
                sub_graph.adj_relations.emplace_back();
                int new_edge_id = (int) sub_graph.edges.size();
                edge_belong_sub[edge_id] = {sub_graph_id, new_edge_id};
                edge_belong_data[{sub_graph_id, new_edge_id}] = edge_id;
                sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
                sub_graph.graph_data[u].adj_edge.push_back(new_edge_id);
                sub_graph.graph_data[v].rev_edge.push_back(new_edge_id);
                sub_graph.update_stime(u);
                sub_graph.update_etime(v);
                lazy_update_r.insert(from);
                lazy_update_r.insert(to);
                return;
            }
        }
        // new sub
        {
            int sub_graph_id = (int) sub_graphs.size();
            sub_graphs.emplace_back();
            auto &sub_graph = sub_graphs.back();
            int u = 0, v = 1, new_edge_id = 0;
            dec_nodes[from].emplace_back(sub_graph_id, u);
            dec_nodes[to].emplace_back(sub_graph_id, v);
            sub_graph.node_seq_mapping[from] = u;
            sub_graph.node_seq_mapping[to] = v;
            sub_graph.graph_data.push_back({u, data_graph.graph_data[from].identity, data_graph.graph_data[from].label, {new_edge_id}});
            sub_graph.graph_data.push_back({v, data_graph.graph_data[to].identity, data_graph.graph_data[to].label, {new_edge_id}});
            sub_graph.adj_relations.emplace_back();
            sub_graph.adj_relations.emplace_back();
            sub_graph.edges.emplace_back(u, v, stime, etime, new_edge_id);
            sub_graph.update_stime(u);
            sub_graph.update_etime(v);
            lazy_update_r.insert(from);
            lazy_update_r.insert(to);
        }
    }

    void delete_edge(int edge_id) {
        const auto &del_edge = data_graph.edges[edge_id];
        int from = del_edge.source_seq, to = del_edge.destination_seq;
        int sub_graph_id = edge_belong_sub[edge_id].first, new_edge_id = edge_belong_sub[edge_id].second;
        auto &sub_graph = sub_graphs[sub_graph_id];
        int u = sub_graph.node_seq_mapping[from], v = sub_graph.node_seq_mapping[to];
        for (int i = 0; i < (int) sub_graph.graph_data[u].adj_edge.size(); ++i) {
            if (sub_graph.graph_data[u].adj_edge[i] == new_edge_id) {
                sub_graph.graph_data[u].adj_edge.erase(sub_graph.graph_data[u].adj_edge.begin() + i);
            }
        }
        for (int i = 0; i < (int) sub_graph.graph_data[v].rev_edge.size(); ++i) {
            if (sub_graph.graph_data[v].rev_edge[i] == new_edge_id) {
                sub_graph.graph_data[v].rev_edge.erase(sub_graph.graph_data[v].rev_edge.begin() + i);
            }
        }
        edge_belong_sub.erase(edge_id);
        edge_belong_data.erase({sub_graph_id, new_edge_id});
        sub_graph.update_stime(u);
        sub_graph.update_etime(v);
        while (sub_graph.max_min_etime[u] > sub_graph.min_max_stime[u]) {
            for (auto rev : sub_graph.graph_data[u].rev_edge) {
                if (sub_graph.edges[rev].end_time > sub_graph.min_max_stime[u]) {
                    delete_edge(edge_belong_data[{sub_graph_id, rev}]);
                    insert_edge(edge_belong_data[{sub_graph_id, rev}]);
                }
            }
            sub_graph.update_etime(u);
        }
        lazy_update_r.insert(from);
        while (sub_graph.max_min_etime[v] > sub_graph.min_max_stime[v]) {
            for (auto adj : sub_graph.graph_data[v].adj_edge) {
                if (sub_graph.edges[adj].start_time < sub_graph.max_min_etime[v]) {
                    delete_edge(edge_belong_data[{sub_graph_id, adj}]);
                    insert_edge(edge_belong_data[{sub_graph_id, adj}]);
                }
            }
            sub_graph.update_stime(v);
        }
        lazy_update_r.insert(to);
    }
};