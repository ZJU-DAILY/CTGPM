#include <bits/stdc++.h>

int main() {
    std::string data_set = "L25";
    std::vector<std::pair<std::string, std::string>> para = {
            {"3", "1.15"}, {"4", "1.15"}, {"5", "1.15"}, {"6", "1.15"}, {"7", "1.15"}, {"8", "1.15"},
            {"6", "1.05"}, {"6", "1.1"}, {"6", "1.2"}, {"6", "1.25"}
    };
    for (const auto &[p1, p2] : para) {
        std::string param = p1 + "-" + p2;
        for (int i = 0; i < 25; ++i) {
            std::string file_path = "../" + data_set + "/" + param + "/" + std::to_string(i) + ".p";
            struct edge {int id, from, to;};
            std::map<int, std::string> nodes;
            std::vector<edge> edges;
            std::ifstream infile(file_path);
            std::string line;
            while (getline(infile, line)) {
                if (line.empty()) break;
                std::stringstream ss(line);
                std::string tp;
                ss >> tp;
                if (tp == "edge") {
                    int from, to;
                    ss >> from >> to;
                    edges.push_back({233, from, to});
                } else {
                    int id;
                    std::string label;
                    ss >> id >> label;
                    nodes[id] = label;
                }
            }
            infile.close();
            std::sort(edges.begin(), edges.end(), [&](const auto &lhs, const auto &rhs) {
                if (lhs.from != rhs.from) return lhs.from < rhs.from;
                return lhs.to < rhs.to;
            });
            for (int i = 0; i < edges.size(); ++i) {
                edges[i].id = i;
            }
            std::ofstream outfile(file_path);
            for (const auto &e : edges) {
                outfile << "e " << e.id << ' ';
                outfile << nodes[e.from] << ' '  << e.from << ' ';
                outfile << nodes[e.to] << ' ' << e.to << '\n';
            }
            std::vector<int> in(edges.size(), 0);
            std::vector<std::vector<int>> adj(edges.size());
            for (int i = 0; i < edges.size(); ++i) {
                for (int j = i + 1; j < edges.size(); ++j) {
                    if (edges[i].to == edges[j].from) {
                        adj[i].push_back(j);
                        in[j]++;
                    }
                    if (edges[j].to == edges[i].from) {
                        adj[j].push_back(i);
                        in[i]++;
                    }
                }
            }
            for (int i = 0; i < edges.size(); ++i) {
                std::vector<int> vis(edges.size(), false);
                vis[i] = true;
                std::queue<int> q;
                q.push(i);
                while (!q.empty()) {
                    int u = q.front();
                    if (i != u) outfile << "b " << i << ' ' << u << '\n';
                    q.pop();
                    for (auto e : adj[u]) {
                        if (!vis[e]) {
                            vis[e] = true;
                            q.push(e);
                        }
                    }
                }
            }
            std::vector<bool> vis(edges.size(), false);
            while (true) {
                int pos = -1;
                for (int i = 0; i < edges.size(); ++i) {
                    if (!vis[i] && in[i] == 0) {
                        pos = i;
                        break;
                    }
                }
                if (pos == -1) break;
                vis[pos] = true;
                std::vector<int> tmp;
                tmp.push_back(pos);
                for (auto e : adj[pos]) in[e]--;
                while (true) {
                    bool flag = false;
                    for (auto e : adj[pos]) {
                        if (!vis[e]) {
                            pos = e;
                            vis[e] = true;
                            tmp.push_back(e);
                            for (auto ee : adj[e]) in[ee]--;
                            flag = true;
                            break;
                        }
                    }
                    if (!flag) break;
                }
                if (tmp.empty()) break;
                outfile << "tc " << tmp.size() << ' ';
                for (int i = 0; i < tmp.size(); ++i) {
                    outfile << tmp[i] << " \n"[i + 1 == tmp.size()];
                }
            }
        }
    }


    return 0;
}
