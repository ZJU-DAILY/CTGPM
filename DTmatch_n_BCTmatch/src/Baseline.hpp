/*
 * Copyright (c) 2021 by Contributors
 * \file Baseline.hpp
 * \date 2021-10
 * \author Xinwei Cai
 */
#pragma once

#include "DataGraph.hpp"
#include "PatternGraph.hpp"
#include "MatchGraph.hpp"

struct Baseline {
    const DataGraph &data_graph;
    const PatternGraph &pattern_graph;

    std::vector<MatchGraph> match_graphs;

    Baseline(const DataGraph &dg,
             const PatternGraph &pg) : data_graph(dg), pattern_graph(pg) {
        //        std::cout << data_graph.graph_data.size() << '\n';
        //        std::cout << pattern_graph.graph_data.size() << '\n';
    }

    int BCTmatch(int l_hop, int S, int A, int k) {
        match_graphs.clear();

        std::list<Path> paths;
        std::vector<std::list<int>> sim_node;
        std::vector<std::vector<std::list<std::list<Path>::iterator>>> sim_adj_paths;

        const auto &data_nodes = data_graph.graph_data;
        const auto &pattern_nodes = pattern_graph.graph_data;

        std::map<std::string, std::list<int>> data_label_seq;
        std::map<std::string, std::set<std::string>> pattern_adj_label;
        for (const auto &node : data_nodes) {
            if (!data_label_seq.count(node.label)) {
                data_label_seq[node.label] = {node.seq};
            } else {
                data_label_seq[node.label].push_back(node.seq);
            }
        }
        for (const auto &node : pattern_nodes) {
            for (const auto &to : node.adj_edge) {
                pattern_adj_label[node.label].insert(pattern_nodes[to].label);
            }
        }

        for (const auto &node : pattern_nodes) {
            auto ptr = data_label_seq.find(node.label);
            if (ptr != data_label_seq.end())
                sim_node.emplace_back(ptr->second);
            else
                sim_node.emplace_back();
        }

        std::map<std::pair<std::string, std::string>, std::list<std::list<Path>::iterator>> ss_edge;
        for (const auto &node : data_nodes) {
            const auto &s_label = node.label;
            if (!pattern_adj_label.count(s_label)) continue;
            const auto &adj_label = pattern_adj_label.find(s_label)->second;
            std::function<void(int, int, int, int)> dfs = [&](int st_time, int en_time, int id, int len) {
                if (len > l_hop) return;
                if (adj_label.count(data_nodes[id].label)) {
                    paths.emplace_back(node.seq, id, st_time, en_time, len);
                    if (ss_edge.count({s_label, data_nodes[id].label})) {
                        ss_edge[{s_label, data_nodes[id].label}].emplace_back(--paths.end());
                    }
                    else {
                        ss_edge[{s_label, data_nodes[id].label}] = {--paths.end()};
                    }
                }
                for (const auto &adj_id : data_nodes[id].adj_edge) {
                    auto &adj_edge = data_graph.edges[adj_id];
                    if (adj_edge.start_time < en_time) continue;
                    if (adj_edge.end_time > A) continue;
                    dfs(st_time, adj_edge.end_time, adj_edge.destination_seq, len + 1);
                }
            };
            for (const auto &edge_id : node.adj_edge) {
                auto &edge = data_graph.edges[edge_id];
                if (edge.start_time < S) continue;
                dfs(edge.start_time, edge.end_time, edge.destination_seq, 1);
            }
        }
        for (const auto &node : pattern_nodes) {
            std::vector<std::list<std::list<Path>::iterator>> tmp_adj_vec;
            for (const auto &edge : node.adj_edge) {
                tmp_adj_vec.emplace_back(ss_edge[{node.label, pattern_nodes[edge].label}]);
            }
            sim_adj_paths.emplace_back(tmp_adj_vec);
        }

        std::vector<int> rand(pattern_nodes.size());
        std::iota(rand.begin(), rand.end(), 0);
        std::shuffle(rand.begin(), rand.end(), std::mt19937(std::random_device()()));

        while (true) {
            bool change = false;
            for (const auto seq : rand) {
                auto &adj_path = sim_adj_paths[seq];
                for (auto ptr = sim_node[seq].begin(); ptr != sim_node[seq].end();) {
                    auto snode = *ptr;
                    auto tmp_ptr = ptr;
                    ptr++;
                    bool satisfy = true;
                    int min_max_start_time = I32_MAX;
                    for (const auto &spaths : adj_path) {
                        bool exits = false;
                        int max_start_time = -1;
                        for (const auto &path_ptr : spaths) {
                            if ((*path_ptr).source_seq == snode) {
                                exits = true;
                                max_start_time = std::max(max_start_time, (*path_ptr).start_time);
                            }
                        }
                        satisfy &= exits;
                        if (max_start_time != -1) min_max_start_time = std::min(min_max_start_time, max_start_time);
                    }
                    int max_min_end_time = -1;
                    for (const auto &rev_pedge : pattern_nodes[seq].rev_edge) {
                        int rev_seq = rev_pedge;
                        for (int edge_i = 0; edge_i < (int) pattern_nodes[rev_seq].adj_edge.size(); ++edge_i) {
                            if (pattern_nodes[rev_seq].adj_edge[edge_i] != seq) continue;
                            auto &spaths = sim_adj_paths[rev_seq][edge_i];
                            bool exits = false;
                            int min_end_time = I32_MAX;
                            for (const auto &path_ptr : spaths) {
                                if ((*path_ptr).destination_seq == snode) {
                                    exits = true;
                                    min_end_time = std::min(min_end_time, (*path_ptr).end_time);
                                }
                            }
                            satisfy &= exits;
                            if (min_end_time != I32_MAX) max_min_end_time = std::max(max_min_end_time, min_end_time);
                        }
                    }
                    if (!satisfy) {
                        //                        std::cout << data_nodes[snode].identity << " out\n";
                        sim_node[seq].erase(tmp_ptr);
                        for (auto & spaths : adj_path) {
                            for (auto it = spaths.begin(); it != spaths.end();) {
                                auto tmp_it = it;
                                ++it;
                                if ((*tmp_it)->source_seq == snode) spaths.erase(tmp_it);
                            }
                        }
                        for (const auto &rev_pedge : pattern_nodes[seq].rev_edge) {
                            int rev_seq = rev_pedge;
                            for (int edge_i = 0; edge_i < (int) pattern_nodes[rev_seq].adj_edge.size(); ++edge_i) {
                                if (pattern_nodes[rev_seq].adj_edge[edge_i] != seq) continue;
                                auto &spaths = sim_adj_paths[rev_seq][edge_i];
                                for (auto it = spaths.begin(); it != spaths.end();) {
                                    auto tmp_it = it;
                                    ++it;
                                    if ((*tmp_it)->destination_seq == snode) spaths.erase(tmp_it);
                                }
                            }
                        }
                        change = true;
                        continue;
                    }
                    for (auto & sapaths : adj_path) {
                        for (auto it = sapaths.begin(); it != sapaths.end();) {
                            auto tmp_it = it;
                            ++it;
                            if ((*tmp_it)->source_seq == snode && (*tmp_it)->start_time < max_min_end_time) {
                                //                                std::cout << data_nodes[snode].identity << ' ' << data_nodes[(*tmp_it)->destination_seq].identity << " erase\n";
                                sapaths.erase(tmp_it);
                                change = true;
                            }
                        }
                    }
                    for (const auto &rev_pedge : pattern_nodes[seq].rev_edge) {
                        int rev_seq = rev_pedge;
                        for (int edge_i = 0; edge_i < (int) pattern_nodes[rev_seq].adj_edge.size(); ++edge_i) {
                            if (pattern_nodes[rev_seq].adj_edge[edge_i] != seq) continue;
                            auto &spaths = sim_adj_paths[rev_seq][edge_i];
                            for (auto it = spaths.begin(); it != spaths.end();) {
                                auto tmp_it = it;
                                ++it;
                                if ((*tmp_it)->destination_seq == snode && (*tmp_it)->end_time > min_max_start_time) {
                                    //                                    std::cout << data_nodes[(*tmp_it)->source_seq].identity << ' ' << data_nodes[snode].identity << " erase\n";
                                    spaths.erase(tmp_it);
                                    change = true;
                                }
                            }
                        }
                    }
                }
            }
            if (!change) break;
        }

        std::set<std::pair<int, int>> vis_st;
        for (const auto dnode : sim_node.front()) {
            if (!vis_st.count({dnode, 0})) {
                MatchGraph match_graph;
                std::function<void(int, int)> dfs = [&](int id, int pid) {
                    vis_st.insert({id, pid});
                    MatchNode new_mnode;
                    new_mnode.seq = id;
                    new_mnode.identity = data_nodes[id].identity;
                    new_mnode.label = data_nodes[id].label;
                    for (int edge_i = 0; edge_i < (int) sim_adj_paths[pid].size(); ++edge_i) {
                        const auto &paths = sim_adj_paths[pid][edge_i];
                        for (const auto &path_ptr : paths) {
                            if ((*path_ptr).source_seq != id) continue;
                            new_mnode.adj_edge.emplace_back(*path_ptr);
                            if (vis_st.count({(*path_ptr).destination_seq, pattern_nodes[pid].adj_edge[edge_i]})) continue;
                            dfs((*path_ptr).destination_seq, pattern_nodes[pid].adj_edge[edge_i]);
                        }
                    }
                    for (const auto &rev_pedge : pattern_nodes[pid].rev_edge) {
                        int rev_seq = rev_pedge;
                        for (int edge_i = 0; edge_i < (int) pattern_nodes[rev_seq].adj_edge.size(); ++edge_i) {
                            if (pattern_nodes[rev_seq].adj_edge[edge_i] != pid) continue;
                            const auto &spaths = sim_adj_paths[rev_seq][edge_i];
                            for (const auto &path_ptr : spaths) {
                                if ((*path_ptr).destination_seq != id) continue;
                                if (vis_st.count({(*path_ptr).source_seq, rev_seq})) continue;
                                dfs((*path_ptr).source_seq, rev_seq);
                            }
                        }
                    }
                    match_graph.graph_data.emplace_back(new_mnode);
                };
                dfs(dnode, 0);
                match_graphs.emplace_back(match_graph);
            }
        }

        for (auto &mg : match_graphs) {
            mg.calculate_score();
        }

        std::sort(match_graphs.begin(), match_graphs.end(),
                  [&](const auto &lhs, const auto &rhs) {
            return lhs.score > rhs.score;
        });
        for (int i = 0; i < std::min(k, (int)match_graphs.size()); ++i) {
            match_graphs[i].remapping();
            // match_graphs[i].print_detail();
        }
        return (int) match_graphs.size();
    }
};