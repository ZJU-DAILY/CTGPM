#ifndef MATCH_PATTERNGRAPH_HPP
#define MATCH_PATTERNGRAPH_HPP

#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Basic.hpp"

struct PatternGraph {
    std::vector<PatternNode> graph_data;
    std::vector<bool> is_root;
    std::vector<bool> is_destination;
    std::map<std::pair<int, int>, std::pair<int, int>> adj_to_rev, rev_to_adj;

    void loader(std::string file_name) {
        graph_data.clear();

        file_name = "../PatternGraphs/" + file_name;
        std::ifstream infile(file_name.c_str());
        if (!infile) {
            std::cout << "open pattern-graph " << file_name << " filed!\n";
            return;
        }
        std::string line;
        while (getline(infile, line)) {
            if (line.empty()) break;
            std::istringstream instr(line);
            std::string op;
            instr >> op;
            if (op == "node") {
                std::string id, label;
                instr >> id >> label;
                graph_data.push_back(PatternNode(std::stoi(id), id, label, {}, {}));
            }
            else if (op == "edge") {
                int src, des;
                std::string _src, _des;
                instr >> _src >> _des;
                src = std::stoi(_src);
                des = std::stoi(_des);
                adj_to_rev[{src, (int)graph_data[src].adj_edge.size()}] = {des, (int)graph_data[des].rev_edge.size()};
                rev_to_adj[{des, (int)graph_data[des].rev_edge.size()}] = {src, (int)graph_data[src].adj_edge.size()};
                graph_data[src].adj_edge.push_back(des);
                graph_data[des].rev_edge.push_back(src);
            }
        }
        is_root.resize(graph_data.size());
        is_destination.resize(graph_data.size());
        std::fill(is_root.begin(), is_root.end(), false);
        std::fill(is_destination.begin(), is_destination.end(), false);
        std::vector<std::pair<int, int>> degree(graph_data.size(), {0, 0}); // in degree && out degree
        for (const auto &node : graph_data) {
            for (const auto &to : node.adj_edge) {
                degree[node.seq].second++;
                degree[to].first++;
            }
        }
        for (int i = 0; i < (int)graph_data.size(); ++i) {
            if (degree[i].first == 0) is_root[i] = true;
            if (degree[i].second == 0) is_destination[i] = true;
        }
//        std::cout << graph_data.size() << '\n';
//        for (auto node : graph_data) {
//            for (auto edge : node.adj_edge) {
//                std::cout << node.seq << ' ' << edge << '\n';
//            }
//        }
        // std::cout << "set pattern-graph " << file_name << " successfully!\n";
        infile.close();
    }
};

#endif //MATCH_PATTERNGRAPH_HPP
