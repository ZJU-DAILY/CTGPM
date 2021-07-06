#ifndef MATCH_RUNNER_H
#define MATCH_RUNNER_H

#include <random>

#include "Fraction.h"
#include "MatchGraph.h"

struct SimRunner {
    const int L;
    const int S, T;
    const DependencyGraph &dep_graph;
    const PatternGraph &pt_graph;
    const std::vector<std::set<int>> &search_spaces;

    std::vector<MatchGraph> match_graphs;

    explicit SimRunner(int _l, int _s, int _t, const DependencyGraph &_depg, const PatternGraph &_pg,
                       const std::vector<std::set<int>> &_ss) : L(_l), S(_s), T(_t), dep_graph(_depg),
                                                                                pt_graph(_pg),
                                                                                search_spaces(_ss) {}

    int operator()(int x = 1) {
        const auto &sub_graphs = dep_graph.sub_graphs;
        const auto &pattern_nodes = pt_graph.graph_data;
        std::vector<Path> paths;
        std::vector<std::map<int, Fraction>> importance(pattern_nodes.size());
        std::vector<std::map<int, std::pair<int, std::vector<std::pair<int, std::set<int>>>>>> sim_nodes_childs(
                pattern_nodes.size()), sim_nodes_parents(pattern_nodes.size());
        // find paths && build childs table
        using Pos = std::pair<int, int>;
        std::vector<std::map<int, std::map<Pos, int>>> candidates(pattern_nodes.size());
        std::map<std::string, int> identify_to_seq;
        DataGraph new_data_graph;
        for (int sub_id = 0; sub_id < sub_graphs.size(); ++sub_id) {
            for (int node_id = 0; node_id < sub_graphs[sub_id].graph_data.size(); ++node_id) {
                for (int j = 0; j < (int) pattern_nodes.size(); ++j) {
                    if (!pt_graph.is_root[j]) continue;
                    const auto &node = sub_graphs[sub_id].graph_data[node_id];
                    if (node.label == pattern_nodes[j].label) {
                        if (!identify_to_seq.count(node.identity)) {
                            identify_to_seq[node.identity] = (int) new_data_graph.graph_data.size();
                            new_data_graph.graph_data.emplace_back();
                            new_data_graph.graph_data.back().identity = node.identity;
                            new_data_graph.graph_data.back().label = node.label;
                            new_data_graph.graph_data.back().seq = (int) new_data_graph.graph_data.size() - 1;
                        }
                        candidates[j][identify_to_seq[node.identity]][{sub_id, node_id}] = S;
                    }
                }
            }
        }
        for (int i = 0; i < (int) pattern_nodes.size(); ++i) {
            if (pt_graph.is_destination[i]) continue;
            for (const auto &piv : candidates[i]) {
                std::vector<std::pair<int, std::set<int>>> adj_paths(pattern_nodes[i].adj_edge.size(), {-1, {}});
                std::vector<std::map<int, std::map<Pos, int>>> tmp_candidates(pattern_nodes.size());
                int pre_paths_size = paths.size();
                for (const auto &u : piv.second) {
                    std::function<void(int, int, Pos, int, bool)> dfs = [&](int stime, int etime, Pos now, int len, bool flag) {
                        if (len > L) return;
                        if (flag) {
                            for (int j = 0; j < (int) pattern_nodes[i].adj_edge.size(); ++j) {
                                const auto &vv = pattern_nodes[i].adj_edge[j];
                                auto &tmp_node = sub_graphs[now.first].graph_data[now.second];
                                if (tmp_node.label == pattern_nodes[vv].label) {
                                    if (!identify_to_seq.count(tmp_node.identity)) {
                                        identify_to_seq[tmp_node.identity] = (int) new_data_graph.graph_data.size();
                                        new_data_graph.graph_data.emplace_back();
                                        new_data_graph.graph_data.back().identity = tmp_node.identity;
                                        new_data_graph.graph_data.back().label = tmp_node.label;
                                        new_data_graph.graph_data.back().seq = (int) new_data_graph.graph_data.size() - 1;
                                    }
                                    paths.emplace_back(piv.first, identify_to_seq[tmp_node.identity], stime, etime, len);
                                    adj_paths[j].second.insert((int) paths.size() - 1);
                                    adj_paths[j].first = std::max(adj_paths[j].first, paths.back().start_time);
                                    if (!tmp_candidates[vv][identify_to_seq[tmp_node.identity]].count(now))
                                        tmp_candidates[vv][identify_to_seq[tmp_node.identity]][now] = paths.back().end_time;
                                    else
                                        tmp_candidates[vv][identify_to_seq[tmp_node.identity]][now] = std::min(
                                                paths.back().end_time,
                                                tmp_candidates[vv][identify_to_seq[tmp_node.identity]][now]);
                                    for (const auto &e : sub_graphs[now.first].adj_relations[now.second]) {
                                        if (!tmp_candidates[vv][identify_to_seq[tmp_node.identity]].count(e))
                                            tmp_candidates[vv][identify_to_seq[tmp_node.identity]][e] = paths.back().end_time;
                                        else
                                            tmp_candidates[vv][identify_to_seq[tmp_node.identity]][e] = std::min(
                                                    paths.back().end_time,
                                                    tmp_candidates[vv][identify_to_seq[tmp_node.identity]][e]);
                                    }
                                }
                            }
                            for (const auto &e : sub_graphs[now.first].adj_relations[now.second]) {
                                dfs(stime, etime, e, len, false);
                            }
                        }
                        for (const auto &adj_edge : sub_graphs[now.first].graph_data[now.second].adj_edge) {
                            if (adj_edge.start_time < etime) continue;
                            if (adj_edge.end_time > T) continue;
                            dfs(stime, adj_edge.end_time, std::make_pair(now.first, adj_edge.destination_seq), len + 1, true);
                        }
                    };
                    for (const auto &edge : sub_graphs[u.first.first].graph_data[u.first.second].adj_edge) {
                        if (edge.start_time < u.second || edge.end_time > T) continue;
                        dfs(edge.start_time, edge.end_time, std::make_pair(u.first.first, edge.destination_seq), 1, true);
                    }
                }
                bool flag = true;
                for (const auto &st : adj_paths) {
                    if (st.second.empty()) {
                        flag = false;
                        break;
                    }
                }
                if (!flag) {
                    while ((int) paths.size() != pre_paths_size) paths.pop_back();
                } else {
                    int min_max_stime = std::numeric_limits<int>::max();
                    for (const auto &st : adj_paths) {
                        min_max_stime = std::min(min_max_stime, st.first);
                    }
                    sim_nodes_childs[i][piv.first] = std::make_pair(min_max_stime, adj_paths);
                    for (int match : pattern_nodes[i].adj_edge) {
                        for (const auto &v : tmp_candidates[match]) {
                            auto &can = candidates[match][v.first];
                            for (const auto &e : v.second) {
                                if (!can.count(e.first))
                                    can[e.first] = e.second;
                                else
                                    can[e.first] = std::min(can[e.first], e.second);
                            }
                        }
                    }
                }
            }
        }
        for (int i = (int) pattern_nodes.size() - 1; i >= 0; --i) {
            if (!pt_graph.is_destination[i]) continue;
            for (const auto &e : candidates[i]) {
                sim_nodes_childs[i][e.first] = {std::numeric_limits<int>::max(), {}};
            }
        }
        // reverse topo order && build parents table
        for (int i = (int) pattern_nodes.size() - 1; i >= 0; --i) {
            if (pt_graph.is_destination[i]) continue;
            for (auto node_ptr = sim_nodes_childs[i].begin(); node_ptr != sim_nodes_childs[i].end();) {
                auto tmp_node_ptr = node_ptr;
                ++node_ptr;
                bool flag = true;
                for (int j = 0; j < (int) pattern_nodes[i].adj_edge.size(); ++j) {
                    auto &edge_set = (*tmp_node_ptr).second.second[j];
                    for (auto edge_ptr = edge_set.second.begin(); edge_ptr != edge_set.second.end();) {
                        auto tmp_edge_ptr = edge_ptr;
                        ++edge_ptr;
                        int to = paths[(*tmp_edge_ptr)].destination_seq, etime = paths[(*tmp_edge_ptr)].end_time;
                        if (!sim_nodes_childs[pattern_nodes[i].adj_edge[j]].count(to) ||
                            etime > sim_nodes_childs[pattern_nodes[i].adj_edge[j]][to].first) {
                            edge_set.second.erase(tmp_edge_ptr);
                        }
                    }
                    // check !empty
                    if (edge_set.second.empty()) {
                        flag = false;
                        break;
                    }
                    // update max_stime
                    for (const auto edge_id : edge_set.second) {
                        edge_set.first = std::max(edge_set.first, paths[edge_id].start_time);
                    }
                }
                if (!flag) { // check !empty
                    sim_nodes_childs[i].erase(tmp_node_ptr);
                    continue;
                }
                // update min_max_stime && build parents table
                if (!sim_nodes_parents[i].count((*tmp_node_ptr).first)) {
                    sim_nodes_parents[i][(*tmp_node_ptr).first] = {-1, std::vector<std::pair<int, std::set<int>>>(pattern_nodes[i].rev_edge.size(), {std::numeric_limits<int>::max(), {}})};
                }
                (*tmp_node_ptr).second.first = std::numeric_limits<int>::max();
                for (int j = 0; j < (int) pattern_nodes[i].adj_edge.size(); ++j) {
                    (*tmp_node_ptr).second.first = std::min((*tmp_node_ptr).second.first,
                                                            (*tmp_node_ptr).second.second[j].first);
                    auto[ii, jj] = (*pt_graph.adj_to_rev.find({i, j})).second;
                    for (const auto edge_id : (*tmp_node_ptr).second.second[j].second) {
                        int to = paths[edge_id].destination_seq, etime = paths[edge_id].end_time;
                        if (!sim_nodes_parents[ii].count(to)) {
                            std::vector<std::pair<int, std::set<int>>> rev_paths(pattern_nodes[ii].rev_edge.size(),
                                                                                 {std::numeric_limits<int>::max(), {}});
                            rev_paths[jj].second.insert(edge_id);
                            rev_paths[jj].first = etime;
                            sim_nodes_parents[ii][to] = {etime, rev_paths};
                        } else {
                            sim_nodes_parents[ii][to].second[jj].second.insert(edge_id);
                            sim_nodes_parents[ii][to].second[jj].first = std::min(
                                    sim_nodes_parents[ii][to].second[jj].first, etime);
                            sim_nodes_parents[ii][to].first = -1;
                            for (const auto &edges : sim_nodes_parents[ii][to].second) {
                                if (edges.second.empty()) continue;
                                sim_nodes_parents[ii][to].first = std::max(sim_nodes_parents[ii][to].first,
                                                                           edges.first);
                            }
                        }
                        importance[ii][to] += Fraction(1, (*tmp_node_ptr).second.second[j].second.size());
                    }
                }
            }
        }
        for (int i = 0; i < (int) pattern_nodes.size(); ++i) {
            if (!pt_graph.is_root[i]) continue;
            for (const auto &e : sim_nodes_childs[i]) {
                sim_nodes_parents[i][e.first] = {-1, {}};
            }
        }
        // topo order && back tracing update
        for (int i = 0; i < (int) pattern_nodes.size(); ++i) {
            if (pt_graph.is_root[i]) continue;
            std::vector<std::pair<int, Fraction>> order(importance[i].begin(), importance[i].end());
//            std::shuffle(order.begin(), order.end(), std::mt19937(std::random_device()()));
            std::sort(order.begin(), order.end(), [](const auto &lhs, const auto &rhs) {
                return rhs.second < lhs.second;
            });
            while (!order.empty() && order.back().second == Fraction()) order.pop_back();
            for (const auto &[node, _] : order) {
                if (!sim_nodes_parents[i].count(node)) continue;
                std::function<void(int, int, int)> update = [&](int v, int u, int tag) {
                    if (v > i || !sim_nodes_parents[v].count(u)) return;
                    std::set<std::tuple<int, int, int>> op_list;
                    bool flag = true;
                    auto update_parents = [&]() {
                        bool res = false;
                        const int stime = sim_nodes_childs[v][u].first;
                        int max_min_etime = -1;
                        for (int j = 0; j < (int) sim_nodes_parents[v][u].second.size(); ++j) {
                            const auto [vv, jj] = (*pt_graph.rev_to_adj.find({v, j})).second;
                            int min_etime = std::numeric_limits<int>::max();
                            auto &edge_set = sim_nodes_parents[v][u].second[j].second;
                            for (auto edge_ptr = edge_set.begin(); edge_ptr != edge_set.end();) {
                                const auto tmp_edge_ptr = edge_ptr;
                                ++edge_ptr;
                                const int from = paths[(*tmp_edge_ptr)].source_seq;
                                if (paths[(*tmp_edge_ptr)].end_time > stime) {
                                    sim_nodes_childs[vv][from].second[jj].second.erase((*tmp_edge_ptr));
                                    edge_set.erase(tmp_edge_ptr);
                                    op_list.insert({vv, from, 0});
                                    res = true;
                                } else {
                                    min_etime = std::min(min_etime, paths[(*tmp_edge_ptr)].end_time);
                                }
                            }
                            if (edge_set.empty()) {
                                flag = false;
                                break;
                            } else {
                                max_min_etime = std::max(max_min_etime, min_etime);
                            }
                        }
                        if (max_min_etime != -1) sim_nodes_parents[v][u].first = max_min_etime;
                        return res;
                    };
                    auto update_childs = [&]() {
                        bool res = false;
                        const int etime = sim_nodes_parents[v][u].first;
                        int min_max_stime = std::numeric_limits<int>::max();
                        for (int j = 0; j < (int) sim_nodes_childs[v][u].second.size(); ++j) {
                            const auto[vv, jj] = (*pt_graph.adj_to_rev.find({v, j})).second;
                            int max_stime = -1;
                            auto &edge_set = sim_nodes_childs[v][u].second[j].second;
                            const int pre_size = edge_set.size();
                            for (auto edge_ptr = edge_set.begin(); edge_ptr != edge_set.end();) {
                                const auto tmp_edge_ptr = edge_ptr;
                                ++edge_ptr;
                                const int to = paths[(*tmp_edge_ptr)].destination_seq;
                                if (paths[(*tmp_edge_ptr)].start_time < etime) {
                                    sim_nodes_parents[vv][to].second[jj].second.erase((*tmp_edge_ptr));
                                    if (vv > i) {
                                        importance[vv][to] -= Fraction(1, pre_size);
                                    }
                                    edge_set.erase(tmp_edge_ptr);
                                    op_list.insert({vv, to, 1});
                                    res = true;
                                } else {
                                    max_stime = std::max(max_stime, paths[(*tmp_edge_ptr)].start_time);
                                }
                            }
                            const int now_size = edge_set.size();
                            if (vv > i && now_size != pre_size) {
                                const Fraction tmp_fra(pre_size - now_size, pre_size * now_size);
                                for (const auto edge : edge_set) {
                                    importance[vv][paths[edge].destination_seq] += tmp_fra;
                                }
                            }
                            if (edge_set.empty()) {
                                flag = false;
                                break;
                            } else {
                                min_max_stime = std::min(min_max_stime, max_stime);
                            }
                        }
                        if (min_max_stime != std::numeric_limits<int>::max())
                            sim_nodes_childs[v][u].first = min_max_stime;
                        return res;
                    };
                    if (tag == 1)
                        update_parents();
                    else
                        update_childs();
                    for (int mask = tag; flag; mask = mask ^ 1) {
                        bool changed = false;
                        if (mask == 0)
                            changed |= update_parents();
                        else
                            changed |= update_childs();
                        if (!changed) break;
                    }
                    // delete node
                    if (!flag) {
                        for (int j = 0; j < (int) sim_nodes_parents[v][u].second.size(); ++j) {
                            const auto[vv, jj] = (*pt_graph.rev_to_adj.find({v, j})).second;
                            auto &edge_set = sim_nodes_parents[v][u].second[j].second;
                            for (const auto &edge : edge_set) {
                                const int from = paths[edge].source_seq;
                                sim_nodes_childs[vv][from].second[jj].second.erase(edge);
                                op_list.insert({vv, from, 0});
                            }
                        }
                        sim_nodes_parents[v].erase(u);
                        for (int j = 0; j < (int) sim_nodes_childs[v][u].second.size(); ++j) {
                            const auto[vv, jj] = (*pt_graph.adj_to_rev.find({v, j})).second;
                            auto &edge_set = sim_nodes_childs[v][u].second[j].second;

                            for (const auto &edge : edge_set) {
                                const int to = paths[edge].destination_seq;
                                sim_nodes_parents[vv][to].second[jj].second.erase(edge);
                                op_list.insert({vv, to, 1});
                            }
                            const int pre_size = edge_set.size();
                            if (vv > i) {
                                for (const auto edge : edge_set) {
                                    importance[vv][paths[edge].destination_seq] -= Fraction(1, pre_size);
                                }
                            }
                        }
                        sim_nodes_childs[v].erase(u);
                    }
                    for (const auto &[uu, vv, ta] : op_list)
                        update(uu, vv, ta);
                };
                update(i, node, 1);
            }
        }

        std::set<std::pair<int, int>> vis_st;
        for (const auto &node : sim_nodes_childs.front()) {
            if (vis_st.count({0, node.first})) continue;
            MatchGraph match_graph;
            std::function<void(int, int)> dfs = [&](int v, int u) {
                vis_st.insert({v, u});
                MatchNode new_node;
                new_node.seq = u;
                new_node.identity = new_data_graph.graph_data[u].identity;
                new_node.label = new_data_graph.graph_data[u].label;
                for (int j = 0; j < (int) sim_nodes_childs[v][u].second.size(); ++j) {
                    const auto &edge_set = sim_nodes_childs[v][u].second[j];
                    for (const auto &edge : edge_set.second) {
                        new_node.adj_edge.push_back(paths[edge]);
                        if (vis_st.count({pattern_nodes[v].adj_edge[j], paths[edge].destination_seq})) continue;
                        dfs(pattern_nodes[v].adj_edge[j], paths[edge].destination_seq);
                    }
                }
                for (int j = 0; j < (int) sim_nodes_parents[v][u].second.size(); ++j) {
                    const auto &edge_set = sim_nodes_parents[v][u].second[j];
                    for (const auto &edge : edge_set.second) {
                        if (vis_st.count({pattern_nodes[v].rev_edge[j], paths[edge].source_seq})) continue;
                        dfs(pattern_nodes[v].rev_edge[j], paths[edge].source_seq);
                    }
                }
                match_graph.graph_data.push_back(std::move(new_node));
            };
            dfs(0, node.first);
            match_graphs.push_back(std::move(match_graph));
            match_graphs.back().remapping();
            match_graphs.back().calculate_score();
        }
        return match_graphs.size();
    }
};

#endif //MATCH_RUNNER_H