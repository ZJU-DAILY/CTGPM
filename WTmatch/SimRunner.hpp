#ifndef MATCH_SIMRUNNER_HPP
#define MATCH_SIMRUNNER_HPP

#include <set>
#include "MatchGraph.hpp"
#include "DisjointSetUnion.hpp"

struct SimRunner {
    const int L;
    const int S, T;
    const DependencyGraph &dep_graph;
    const PatternGraph &pt_graph;

    std::vector<MatchGraph> match_graphs;

    explicit SimRunner(int _l, int _s, int _t, const DependencyGraph &_depg, const PatternGraph &_pg):
        L(_l), S(_s), T(_t), dep_graph(_depg), pt_graph(_pg) {}

    int operator()() {
        const auto &pattern_nodes = pt_graph.graph_data;
        const auto &dep_nodes = dep_graph.graph_data;
        const auto &rea = dep_graph.relations;

        std::vector<std::set<int>> candidates(pattern_nodes.size());
        std::vector<std::map<int, std::vector<std::set<int>>>> sim_nodes_childs(pattern_nodes.size());
        std::vector<std::map<int, std::vector<std::set<int>>>> sim_nodes_parents(pattern_nodes.size());

        // build candidates
        for (int i = 0; i < pattern_nodes.size(); ++i) {
            auto ptr = dep_graph.label_seqs.find(pattern_nodes[i].label);
            if (ptr != dep_graph.label_seqs.end()) {
                candidates[i].insert((ptr->second).begin(), (ptr->second).end());
            }
        }
        // rev topology-order
        for (int i = (int) pattern_nodes.size() - 1; i >= 0; --i) {
            if (pt_graph.is_destination[i]) {
                for (const auto u : candidates[i])
                    sim_nodes_childs[i].insert({u, {}});
                continue;
            }
            for (const auto u : candidates[i]) {
                std::vector<std::set<int>> tmp_child(pattern_nodes[i].adj_edge.size());
                std::function<void(int, int, int)> dfs = [&](int now, int len, bool flag) {
                    if (len > L) return;
                    if (flag) {
                        auto &tmp_node = dep_nodes[now];
                        for (int j = 0; j < (int) pattern_nodes[i].adj_edge.size(); ++j) {
                            const auto &vv = pattern_nodes[i].adj_edge[j];
                            if (tmp_node.label == pattern_nodes[vv].label && sim_nodes_childs[vv].count(now)) {
                                tmp_child[j].insert(now);
                                for (const auto &e : rea[now]) {
                                    if (sim_nodes_childs[vv].count(e)) tmp_child[j].insert(e);
                                }
                            }
                        }
                        for (const auto &e : rea[now]) {
                            dfs(e, len, false);
                        }
                    }
                    for (const auto &edge : dep_nodes[now].adj_edge) {
                        if (edge.start_time < S || edge.end_time > T) continue;
                        dfs(edge.destination_seq, len + 1, true);
                    }
                };
                for (const auto &edge : dep_nodes[u].adj_edge) {
                    if (edge.start_time < S || edge.end_time > T) continue;
                    dfs(edge.destination_seq, 1, true);
                }
                for (const auto &e : rea[u]) {
                    dfs(e, 0, false);
                }
                bool flag = true;
                for (auto &st : tmp_child) {
                    if (st.empty()) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    sim_nodes_childs[i][u] = tmp_child;
                    if (!sim_nodes_parents[i].count(u)) {
                        sim_nodes_parents[i][u] = std::vector<std::set<int>>(pattern_nodes[i].rev_edge.size());
                    }
                    for (int j = 0; j < pattern_nodes[i].adj_edge.size(); ++j) {
                        const auto [ii, jj] = (*pt_graph.adj_to_rev.find({i, j})).second;
                        for (const auto v : sim_nodes_childs[i][u][j]) {
                            if (!sim_nodes_parents[ii].count(v)) {
                                sim_nodes_parents[ii][v] = std::vector<std::set<int>>(pattern_nodes[ii].rev_edge.size());
                            }
                            sim_nodes_parents[ii][v][jj].insert(u);
                        }
                    }
                }
            }
        }

        // topology order && back tracing update
        for (int i = 0; i < (int) pattern_nodes.size(); ++i) {
            if (pt_graph.is_root[i]) continue;
            std::function<void(int, int)> update = [&](int v, int u) {
                if (v > i || !sim_nodes_parents[v].count(u)) return;
                std::set<std::pair<int, int>> op_list;
                bool flag = true;
                auto check = [&]() {
                    if (!pattern_nodes[i].adj_edge.empty() && !sim_nodes_childs[v].count(u)) return false;
                    for (const auto &st : sim_nodes_childs[v][u]) if (st.empty()) return false;
                    if (!pattern_nodes[i].rev_edge.empty() && !sim_nodes_parents[v].count(u)) return false;
                    for (const auto &st : sim_nodes_parents[v][u]) if (st.empty()) return false;
                    return true;
                };
                flag &= check();
                if (!flag) {
                    for (int j = 0; j < sim_nodes_childs[v][u].size(); ++j) {
                        const auto [vv, jj] = (*pt_graph.adj_to_rev.find({v, j})).second;
                        for (const auto to : sim_nodes_childs[v][u][j]) {
                            if (sim_nodes_parents[vv].count(to)) {
                                sim_nodes_parents[vv][to][jj].erase(u);
                                op_list.insert({vv, to});
                            }
                        }
                    }
                    sim_nodes_childs[v].erase(u);
                    for (int j = 0; j < sim_nodes_parents[v][u].size(); ++j) {
                        const auto [vv, jj] = (*pt_graph.rev_to_adj.find({v, j})).second;
                        for (const auto from : sim_nodes_parents[v][u][j]) {
                            if (sim_nodes_childs[vv].count(from)) {
                                sim_nodes_childs[vv][from][jj].erase(u);
                                op_list.insert({vv, from});
                            }
                        }
                    }
                    sim_nodes_parents[v].erase(u);
                }
                for (const auto &[vv, uu] : op_list) {
                    update(vv, uu);
                }
            };
            std::vector<int> tmp;
            for (const auto &e : sim_nodes_childs[i]) {
                tmp.push_back(e.first);
            }
            for (const auto &e : tmp) {
                update(i, e);
            }
        }
        for (int i = 0; i < sim_nodes_childs.size(); ++i) {
            for (auto ptr = sim_nodes_childs[i].begin(); ptr != sim_nodes_childs[i].end();) {
                auto tmp_ptr = ptr;
                ptr++;
                if (!sim_nodes_parents[i].count(tmp_ptr->first)) {
                    sim_nodes_childs[i].erase(tmp_ptr);
                }
            }
        }

        // build match-graph
        std::map<std::string, int> identify_to_seq;
        for (const auto &v : sim_nodes_childs) {
            for (const auto &node : v) {
                if (!identify_to_seq.count(dep_nodes[node.first].identity)) {
                    int sz = identify_to_seq.size();
                    identify_to_seq[dep_nodes[node.first].identity] = sz;
                }
            }
        }
        std::vector<MatchNode> match_nodes(identify_to_seq.size());
        for (int i = 0; i < pattern_nodes.size(); ++i) {
            auto &can = sim_nodes_childs[i];
            for (auto ptr_i = can.begin(), ptr_j = can.begin(); ptr_i != can.end(); ptr_i = ptr_j) {
                int u = ptr_i->first;
                MatchNode new_node;
                new_node.seq = identify_to_seq[dep_nodes[u].identity];
                new_node.identity = dep_nodes[u].identity;
                new_node.label = dep_nodes[u].label;
                while (ptr_j != can.end() && dep_nodes[u].identity == dep_nodes[ptr_j->first].identity) ++ptr_j;
                int st = u;
                while (st < dep_nodes.size() && dep_nodes[st].identity == dep_nodes[u].identity) {
                    std::function<void(int, int, int, int, bool)> search = [&](int stime, int etime, int node, int len, bool tag) {
                        if (len > L) return;
                        if (tag) {
                            bool flag = false;
                            for (const auto &st : sim_nodes_childs[i][u]) {
                                if (st.count(node)) flag = true;
                            }
                            for (const auto &e : rea[node]) {
                                for (const auto &st : sim_nodes_childs[i][u]) {
                                    if (st.count(e)) flag = true;
                                }
                            }
                            if (flag) {
                                new_node.adj_edge.emplace_back(new_node.seq, identify_to_seq[dep_nodes[node].identity], stime, etime, len);
                            }
                            for (const auto &e : rea[node]) {
                                search(stime, etime, e, len, false);
                            }
                        }
                        for (const auto &edge : dep_nodes[node].adj_edge) {
                            search(stime, edge.end_time, edge.destination_seq, len + 1, true);
                        }
                    };
                    for (const auto &edge : dep_nodes[st].adj_edge) {
                        search(edge.start_time, edge.end_time, edge.destination_seq, 1, true);
                    }
                    st++;
                }
                match_nodes[new_node.seq] = new_node;
            }
        }
        DisjointSetUnion dsu(match_nodes.size());
        for (const auto &e : match_nodes) {
            for (const auto &edge : e.adj_edge) {
                int fx = dsu.find(edge.source_seq);
                int fy = dsu.find(edge.destination_seq);
                if (fx != fy) {
                    dsu.fa[fx] = fy;
                }
            }
        }
        std::map<int, std::set<int>> tmp_set;
        for (int i = 0; i < dsu.fa.size(); ++i) {
            tmp_set[dsu.find(i)].insert(i);
        }
        for (const auto &pist : tmp_set) {
            MatchGraph mg;
            for (const auto &e : pist.second) {
                mg.graph_data.push_back(std::move(match_nodes[e]));
            }
            mg.remapping();
            mg.calculate_score();
            match_graphs.push_back(std::move(mg));
        }
        std::sort(match_graphs.begin(), match_graphs.end(),
                  [&](const auto &lhs, const auto &rhs) {
                      return lhs.score > rhs.score;
                  });
        for (auto &mp : match_graphs) {
            // mp.print_detail();
        }
        return match_graphs.size();
    }
};

#endif //MATCH_SIMRUNNER_HPP
