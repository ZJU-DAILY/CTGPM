#include <bits/stdc++.h>

int main() {
    std::string file_name = "../wiki-talk";
    std::ifstream infile(file_name + ".data");
    std::string line;
    struct Edge { int u, v, st, et; std::string al, bl; };
    std::vector<Edge> edges;
    std::map<int, std::string> mappping;
    while (getline(infile, line)) {
        std::stringstream in(line);
        int a, b, st, et;
        std::string al, bl;
        in >> a >> b >> st >> et >> al >> bl;
        edges.push_back({a, b, st, et, al, bl});
    }
    infile.close();
    std::ofstream outfile(file_name + ".data");
    std::sort(edges.begin(), edges.end(), [&](const auto &lhs, const auto &rhs) {
        return lhs.st < rhs.st;
    });
    for (const auto &e : edges) {
        outfile << e.u << ' ' << e.v << ' ' << e.st << ' ' << e.et << ' ' << e.al << ' ' << e.bl << '\n';
    }
    outfile.close();
    return 0;
}
