#ifndef MATCH_DISJOINTSETUNION_HPP
#define MATCH_DISJOINTSETUNION_HPP

struct DisjointSetUnion {
    std::vector<int> fa;

    explicit DisjointSetUnion(int sz) {
        fa.resize(sz);
        std::iota(fa.begin(), fa.end(), 0);
    }

    int find(int x) {
        return x == fa[x] ? x : fa[x] = find(fa[x]);
    }
};

#endif //MATCH_DISJOINTSETUNION_HPP
