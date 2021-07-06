#include <bits/stdc++.h>

int main() {
    auto seed = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto rand_fun = [&](int a, int b) { return std::uniform_int_distribution<>(a, b - 1)(seed); };

    std::vector<std::pair<std::string, std::string>> dts = {
            {"1250", "1.15"},
            {"500", "1.15"}, {"750", "1.15"}, {"1000", "1.15"}, {"1500", "1.15"}, {"1750", "1.15"}, {"2000", "1.15"},
            {"1250", "1.05"}, {"1250", "1.1"}, {"1250", "1.2"}, {"1250", "1.25"},
    };
    for (auto [_node, _alpha] : dts) {
        std::cout << _node << ' ' << _alpha << '\n';
        int node = std::stoi(_node) * 1000;
        double alpha = std::stod(_alpha);
        int label_num = 20;
        int end_time = 87600; // 5*365*24*60
        int edge = std::pow(node, alpha);
        std::vector<std::string> labels(node);
        for (auto &la : labels) {
            la = "L" + std::to_string(rand_fun(0, label_num));
        }
        std::vector<int> max_end(node, 0);
        std::string path = "../DataGraphs/" + _node + "-" + _alpha + ".data";
        std::ofstream ofile(path);
        for (int i = 0; i < edge;) {
            int from = rand_fun(0, node);
            int to = rand_fun(0, node);
            if (from == to) continue;
            ++i;
            int time = rand_fun(0, end_time);
            ofile << from << ' ' << to << ' ';
            ofile << time << ' ' << time + 1 << ' ';
            ofile << labels[from] << ' ' << labels[to] << '\n';
        }
        ofile.close();
    }
    return 0;
}
//#include <bits/stdc++.h>
//
//int main() {
//    auto seed = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
//    auto rand_fun = [&](int a, int b) { return std::uniform_int_distribution<>(a, b - 1)(seed); };
//    std::cout << "cin node:";
//    int node;
//    std::cin >> node;
//    std::cout << "cin alpha:";
//    double alpha;
//    std::cin >> alpha;
//    int block = 100;
//    int label_num = 25;
//    int end_time = 2628000; // 5*365*24*60
//    int edge = std::pow(node, alpha);
//    std::vector<std::string> labels(node);
//    for (auto &la : labels) {
//        la = "L" + std::to_string(rand_fun(0, label_num));
//    }
//    std::vector<int> max_end(node, 0);
//    std::ofstream ofile("DataGraphs/SY6.data");
//    for (int i = 0; i < block; ++i) {
//        int node_low = node / block * i, node_hi = (i == block - 1 ? node : node / block * (i + 1));
//        int edge_num = (edge + block - 1) / block;
//        for (int j = 0; j < edge_num;) {
//            int from = rand_fun(node_low, node_hi);
//            int to = rand_fun(node_low, node_hi);
//            if (from == to) continue;
//            ++j;
//            int time = rand_fun(max_end[from], std::max(max_end[from] + 1, end_time));
//            max_end[to] = std::max(max_end[to], time + 1);
//            ofile << from << ' ' << to << ' ';
//            ofile << time << ' ' << time + 1 << ' ';
//            ofile << labels[from] << ' ' << labels[to] << '\n';
//        }
//    }
//    ofile.close();
//    return 0;
//}
//#include <bits/stdc++.h>
//
//int main() {
//    auto seed = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
//    auto rand_fun = [&](int a, int b) { return std::uniform_int_distribution<>(a, b - 1)(seed); };
//    std::ifstream infile("cit-ph.tmp");
//    std::vector<std::pair<std::pair<int, int>, int>> edges;
//    int u, v, time;
//    std::map<int, std::string> labels;
//    int num = 30;
//    while (infile >> u >> v >> time) {
//        edges.push_back({{u, v}, time});
//        if (!labels.count(u)) {
//            labels[u] = "L" + std::to_string(rand_fun(0, num));
//        }
//        if (!labels.count(v)) {
//            labels[v] = "L" + std::to_string(rand_fun(0, num));
//        }
//    }
//    infile.close();
//    std::ofstream outfile("cit-ph.data");
//    for (auto e : edges) {
//        outfile << e.first.first << ' ' << e.first.second << ' ';
//        outfile << e.second << ' ' << e.second + 10 << ' ';
//        outfile << labels[e.first.first] << ' ' << labels[e.first.second] << '\n';
//    }
//    outfile.close();
//}