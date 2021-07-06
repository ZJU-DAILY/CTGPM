#include <chrono>

#include "PatternGraph.h"
#include "DataGraph.h"
#include "DependencyGraph.h"
#include "SimBalancer.h"
#include "Baseline.h"

int main() {
    auto time_cost = [](const auto st, const auto en) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(en - st).count();
    };

    std::string way = "opt";

    int L = 1;
    int THREAD_SIZE = 96;
    int K = 3, S = 0;
    int T = std::numeric_limits<int>::max() - 1;

    DataGraph data_graph;
    data_graph.loader("highschool.data");

    DependencyGraph dep_graph(data_graph);
    if (way != "baseline") {
        auto time_st = std::chrono::system_clock::now();
        dep_graph.decompose();
        auto time_en = std::chrono::system_clock::now();
        int node = 0, rea = 0;
        for (const auto &e : dep_graph.sub_graphs) {
            node += (int) e.graph_data.size();
            rea += (int) e.topo_order_relations.size();
        }
        std::cout << dep_graph.sub_graphs.size() << ' ' << node << ' ' << rea << '\n';
        std::cout << "build dep cost(ms):" << time_cost(time_st, time_en) << '\n';
    }


    for (int iii = 9; iii <= 9; ++iii) {
        PatternGraph pattern_graph;
        std::string pathp = "highschool/3-1.15/" + std::to_string(iii) + ".p";
        pattern_graph.loader(pathp);
        std::ofstream of("../Results/" + way + "aaaaa.re");
        if (way == "baseline") {
            Baseline bs(data_graph, pattern_graph);
            auto time_st = std::chrono::system_clock::now();
            bs.match2(L, S, T, K);
            auto time_en = std::chrono::system_clock::now();
            std::cout << "simulation cost(ms): " << time_cost(time_st, time_en) << "\n";
            of << "baseline simulation cost(ms): " << time_cost(time_st, time_en) << "\n";
            for (int i = 0; i < std::min(K, (int) bs.match_graphs.size()); ++i) {
                // bs.match_graphs[i].print_detail();
            }
        } else {
            std::vector<int> all_space(dep_graph.sub_graphs.size());
            std::iota(all_space.begin(), all_space.end(), 0);
            std::set<int> sp(all_space.begin(), all_space.end());
            auto time_st = std::chrono::system_clock::now();
            SimBalancer simbl(THREAD_SIZE, L, K, S, T, dep_graph, pattern_graph, {sp});
            simbl.launch();
            auto time_en = std::chrono::system_clock::now();
            std::cout << "simulation cost(ms): " << time_cost(time_st, time_en) << "\n";
            of << "opt simulation cost(ms): " << time_cost(time_st, time_en) << "\n";
            for (int i = 0; i < std::min(K, (int) simbl.match_graphs.size()); ++i) {
                simbl.match_graphs[i].print_detail();
            }
        }
    }
    return 0;
}
