#ifndef MATCH_DATAGRAPH_HPP
#define MATCH_DATAGRAPH_HPP

#include <map>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <algorithm>
#include <functional>
#include <queue>
#include <numeric>
#include "Basic.hpp"

struct DataGraph {
    std::vector<DataNode> graph_data;
    std::map<std::string, int> identity_to_seq;

    void loader(std::string file_name) {
        graph_data.clear();
        identity_to_seq.clear();

        file_name = "../DataGraphs/" + file_name;
        std::ifstream infile(file_name.c_str());
        if (!infile) {
            std::cout << "open data-graph " << file_name << " filed!\n";
            return;
        }
        std::string line;
        while (getline(infile, line)) {
            if (line.empty()) break;
            std::istringstream instr(line);
            std::vector<std::string> id(2), label(2);
            std::string stime, etime;
            // instr >> id[0] >> id[1] >> label[0] >> label[1] >> stime >> etime;
            instr >> id[0] >> id[1] >> stime >> etime >> label[0] >> label[1];
            for (int i = 0; i < 2; ++i) {
                if (identity_to_seq.find(id[i]) == identity_to_seq.end()) {
                    DataNode new_node(graph_data.size(), id[i], label[i], {});
                    graph_data.push_back(new_node);
                    identity_to_seq[new_node.identity] = new_node.seq;
                }
            }
            Edge new_edge(identity_to_seq[id[0]], identity_to_seq[id[1]],
                          std::stoi(stime), std::stoi(etime));
            graph_data[identity_to_seq[id[0]]].adj_edge.push_back(new_edge);
        }
        for (auto &node : graph_data) {
            std::sort(node.adj_edge.begin(), node.adj_edge.end(), [](const Edge &lhs, const Edge &rhs) {
                if (lhs.start_time == rhs.start_time) return lhs.end_time < rhs.end_time;
                return lhs.start_time < rhs.start_time;
            });
        }
//        std::cout << graph_data.size() << '\n';
        std::cout << "set data-graph " << file_name << " successfully!\n";
        infile.close();
    }

    void print_detail() {
        std::cout << "nodes\n";
        for (const auto &node : graph_data) {
            std::cout << node.seq << ' ' << node.label << '\n';
        }
        std::cout << "edge\n";
        for (const auto &node : graph_data) {
            for (const auto &edge : node.adj_edge) {
                std::cout << edge.source_seq << ' ' << edge.destination_seq << ' ';
                std::cout << edge.start_time << ' ' << edge.end_time << '\n';
            }
        }
    }

    int cal_diameter() {
        int res = -1;
        const int sz = graph_data.size();
        std::vector<std::vector<int>> adj(sz);
        for (const auto &node : graph_data) {
            for (const auto &edge : node.adj_edge)
                adj[edge.source_seq].push_back(edge.destination_seq);
        }
        for (auto &v : adj) {
            std::sort(v.begin(), v.end());
            v.erase(std::unique(v.begin(), v.end()), v.end());
        }
        std::queue<int> que;
        std::vector<int> dist(graph_data.size());
        std::vector<int> order(sz);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), std::mt19937(std::random_device()()));
        for (int j = 0; j < 1000; ++j) {
            int i = order[j];
            std::fill(dist.begin(), dist.end(), -1);
            dist[i] = 0;
            que.push(i);
            while (!que.empty()) {
                int u = que.front();
                que.pop();
                for (const auto e : adj[u]) {
                    if (dist[e] != -1) continue;
                    que.push(e);
                    dist[e] = dist[u] + 1;
                }
            }
            for (const auto &e : dist)
                res = std::max(res, e);
        }
        return res;
    }
};

#endif //MATCH_DATAGRAPH_HPP
