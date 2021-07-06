#include <bits/stdc++.h>

int main() {
    auto seed = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto rand_fun = [&](int a, int b) { return std::uniform_int_distribution<>(a, b - 1)(seed); };
    std::ifstream infile("../DataGraphs/superuser.data");
    std::vector<std::pair<std::pair<int, int>, int>> edges;
    int u, v, stime, etime;
    std::string la, lb;
    std::map<int, std::string> labels;
    int num = 3000;
    while (infile >> u >> v >> stime >> etime >> la >> lb) {
        edges.push_back({{u, v}, stime});
        if (!labels.count(u)) {
            labels[u] = "L" + std::to_string(rand_fun(0, num));
        }
        if (!labels.count(v)) {
            labels[v] = "L" + std::to_string(rand_fun(0, num));
        }
    }
    infile.close();
    std::ofstream outfile("../DataGraphs/ML3000.data");
    for (auto e : edges) {
        outfile << e.first.first << ' ' << e.first.second << ' ';
        outfile << e.second << ' ' << e.second + 10 << ' ';
        outfile << labels[e.first.first] << ' ' << labels[e.first.second] << '\n';
    }
    outfile.close();
}