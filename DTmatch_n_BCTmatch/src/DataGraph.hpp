/*
 * Copyright (c) 2021 by Contributors
 * \file DataGraph.hpp
 * \date 2021-10
 * \author Xinwei Cai
 */
#pragma once

#include "Base.hpp"

struct DataGraph {
    int max_time, min_time;
    std::vector<DataNode> graph_data;
    std::map<std::string, int> identity_to_seq;

    std::vector<Edge> edges;
    std::vector<int> belong_which_sub;


    void loader(std::string file_name) {
        max_time = I32_MIN;
        min_time = I32_MAX;
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
            if (id[0] == id[1]) continue;
            min_time = std::min(min_time, std::stoi(stime));
            max_time = std::max(max_time, std::stoi(etime));
            for (int i = 0; i < 2; ++i) {
                if (identity_to_seq.find(id[i]) == identity_to_seq.end()) {
                    DataNode new_node(graph_data.size(), id[i], label[i], {});
                    graph_data.push_back(new_node);
                    identity_to_seq[new_node.identity] = new_node.seq;
                }
            }
            int edge_seq = (int) edges.size();
            edges.emplace_back(identity_to_seq[id[0]], identity_to_seq[id[1]],
                               std::stoi(stime), std::stoi(etime), edge_seq);
            graph_data[identity_to_seq[id[0]]].adj_edge.push_back(edge_seq);
            graph_data[identity_to_seq[id[1]]].rev_edge.push_back(edge_seq);
        }
//        for (auto &node : graph_data) {
//            std::sort(node.adj_edge.begin(), node.adj_edge.end(), [&](const auto &lhs, const auto &rhs) {
//                if (edges[lhs].start_time == edges[rhs].start_time) return edges[lhs].end_time < edges[rhs].end_time;
//                return edges[lhs].start_time < edges[rhs].start_time;
//            });
//        }
        //        std::cout << graph_data.size() << '\n';
        std::cout << "set data-graph " << file_name << " successfully!\n";
        infile.close();
    }

    void loader(std::vector<std::string> data_lines) {
        for (auto &line : data_lines) {
            std::istringstream instr(line);
            std::vector<std::string> id(2), label(2);
            std::string stime, etime;
            // instr >> id[0] >> id[1] >> label[0] >> label[1] >> stime >> etime;
            instr >> id[0] >> id[1] >> stime >> etime >> label[0] >> label[1];
            if (id[0] == id[1]) continue;
            min_time = std::min(min_time, std::stoi(stime));
            max_time = std::max(max_time, std::stoi(etime));
            for (int i = 0; i < 2; ++i) {
                if (identity_to_seq.find(id[i]) == identity_to_seq.end()) {
                    DataNode new_node(graph_data.size(), id[i], label[i], {});
                    graph_data.push_back(new_node);
                    identity_to_seq[new_node.identity] = new_node.seq;
                }
            }
            int edge_seq = (int) edges.size();
            edges.emplace_back(identity_to_seq[id[0]], identity_to_seq[id[1]],
                               std::stoi(stime), std::stoi(etime), edge_seq);
            graph_data[identity_to_seq[id[0]]].adj_edge.push_back(edge_seq);
            graph_data[identity_to_seq[id[1]]].rev_edge.push_back(edge_seq);
        }
    }

    int insert(std::string &line) {
        std::istringstream instr(line);
        std::vector<std::string> id(2), label(2);
        std::string stime, etime;
        // instr >> id[0] >> id[1] >> label[0] >> label[1] >> stime >> etime;
        instr >> id[0] >> id[1] >> stime >> etime >> label[0] >> label[1];
        min_time = std::min(min_time, std::stoi(stime));
        max_time = std::max(max_time, std::stoi(etime));
        for (int i = 0; i < 2; ++i) {
            if (identity_to_seq.find(id[i]) == identity_to_seq.end()) {
                DataNode new_node(graph_data.size(), id[i], label[i], {});
                graph_data.push_back(new_node);
                identity_to_seq[new_node.identity] = new_node.seq;
            }
        }
        int edge_seq = (int) edges.size();
        edges.emplace_back(identity_to_seq[id[0]], identity_to_seq[id[1]],
                           std::stoi(stime), std::stoi(etime), edge_seq);
        graph_data[identity_to_seq[id[0]]].adj_edge.push_back(edge_seq);
        graph_data[identity_to_seq[id[1]]].rev_edge.push_back(edge_seq);
        return edge_seq;
    }

    std::pair<int, int> del(std::string &line) {
        return {0, 1};
    }



    void print_detail() {
        std::cout << "nodes\n";
        for (const auto &node : graph_data) {
            std::cout << node.seq << ' ' << node.label << '\n';
        }
        std::cout << "edge\n";
        for (const auto &node : graph_data) {
            for (const auto &edge_id : node.adj_edge) {
                const auto &edge = edges[edge_id];
                std::cout << edge.source_seq << ' ' << edge.destination_seq << ' ';
                std::cout << edge.start_time << ' ' << edge.end_time << '\n';
            }
        }
    }

};