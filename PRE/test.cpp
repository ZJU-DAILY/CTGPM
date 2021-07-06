#include <bits/stdc++.h>

int main() {
    std::ifstream infile("../DataGraphs/highschool.data");
    std::map<int, int> mapping;
    std::vector<std::pair<int, int>> edges;
    int id1, id2, st, et;
    std::string l1, l2;
    while (infile >> id1 >> id2 >> st >> et >> l1 >> l2) {
        if (!mapping.count(id1)) {
            int sz = mapping.size();
            mapping[id1] = sz;
        }
        if (!mapping.count(id2)) {
            int sz = mapping.size();
            mapping[id2] = sz;
        }
        edges.push_back({mapping[id1], mapping[id2]});
    }
    std::vector<std::vector<int>> adj(mapping.size());
    for (auto e : edges) {
        adj[e.first].push_back(e.second);
        // adj[e.second].push_back(e.first);
    }
    std::cout << mapping.size() << '\n';
    auto bfs3 = [&](int root) {
        std::vector<int> vis(adj.size(), 0);
        std::queue<int> que;
        que.push(root);
        vis[root] = 1;
        for (int j = 0; j < 3; ++j) {
            int sz = que.size();
            for (int i = 0; i < sz; ++i) {
                int u = que.front();
                que.pop();
                for (auto e : adj[u]) {
                    if (vis[e] == 0) {
                        vis[e] = 1;
                        que.push(e);
                    }
                }
            }
        }
        int res = 0;
        for (auto e : vis) res += (e == 1 ? 1 : 0);
        return res;
    };
    auto bfs = [&](int root) {
        std::vector<int> vis(adj.size(), 0);
        std::queue<int> que;
        que.push(root);
        vis[root] = 1;
        while (!que.empty()) {
            int u = que.front();
            que.pop();
            for (auto e : adj[u]) {
                if (vis[e] == 0) {
                    vis[e] = 1;
                    que.push(e);
                }
            }
        }
        int res = 0;
        for (auto e : vis) res += (e == 1 ? 1 : 0);
        return res;
    };
    auto seed = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto rand_fun = [&](int a, int b) { return std::uniform_int_distribution<>(a, b - 1)(seed); };
    double score = 0.;
    int times = adj.size() ;
    for (int i = 0; i < times; ++i) {
        int rnd = i;
        score += bfs3(rnd) * 100.0 / bfs(rnd);
        std::cout << bfs3(rnd) << ' ' << bfs(rnd) << '\n';
    }
    std::cout << score / times;
    // std::cout << cnt / 100.0 / adj.size() / times << '\n';
}