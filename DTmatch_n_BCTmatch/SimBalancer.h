#ifndef MATCH_BALANCER_H
#define MATCH_BALANCER_H

#include <atomic>
#include <thread>
#include "SimRunner.h"

struct SimBalancer {
    const int THREAD_SIZE;
    const int L, K;
    const int S, T;
    const DependencyGraph &dep_graph;
    const PatternGraph &pt_grpah;
    const std::vector<std::set<int>> &search_spaces;

    std::vector<MatchGraph> match_graphs;

    explicit SimBalancer(int _ts, int _l, int _k, int _s, int _t, const DependencyGraph &_depg, const PatternGraph &_pg,
                         const std::vector<std::set<int>> &_ss) : THREAD_SIZE(_ts), L(_l), K(_k), S(_s),
                                                                                  T(_t), dep_graph(_depg),
                                                                                  pt_grpah(_pg), search_spaces(_ss) {}

    int launch() {
        match_graphs.clear();

        int ts = std::min(THREAD_SIZE, (int) search_spaces.size());
        std::vector<SimRunner> runners;
        runners.reserve(ts);
        for (int i = 0; i < ts; ++i) {
            runners.emplace_back(L, S, T, dep_graph, pt_grpah, search_spaces);
        }
        std::atomic_int p(0);
        auto get_job = [&]() {
            return std::atomic_fetch_add_explicit(&p, 1, std::memory_order_relaxed);
        };
        auto run = [&](int index) {
            for (int x; (x = get_job()) < (int) search_spaces.size(); runners[index](x));
        };
        std::vector<std::thread> threads(ts);
        for (int i = 0; i < ts; ++i) {
            threads[i] = std::thread(run, i);
        }
        for (auto &thread : threads)
            thread.join();

        for (const auto &runner : runners) {
            for (const auto &match_graph : runner.match_graphs) {
                match_graphs.push_back(match_graph);
            }
        }
        std::sort(match_graphs.begin(), match_graphs.end(), [&](const auto &lhs, const auto &rhs) {
            return lhs.score > rhs.score;
        });
        return match_graphs.size();
    }
};

#endif //MATCH_BALANCER_H
