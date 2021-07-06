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
    int THREAD_SIZE = 48;
    int K = 3, S = 0;
    int T = std::numeric_limits<int>::max() - 1;

    std::vector<std::string> ways = {"opt", "baseline"};// OPT->DTmatch, baseline->BCTmatch
    std::vector<std::pair<std::string, std::string>> dts = {
            {"500", "1.15"}, {"750", "1.15"}, {"1000", "1.15"},
            {"1250", "1.15"},
            {"1500", "1.15"}, {"1750", "1.15"}, {"2000", "1.15"},
            {"1250", "1.05"}, {"1250", "1.1"}, {"1250", "1.2"}, {"1250", "1.25"},
    };
    for (int L = 1; L <= 1; ++L) {
        for (const auto &[_node, _alpha] : dts) {
            std::string data_set = _node + "-" + _alpha;
            DataGraph data_graph;
            data_graph.loader(data_set + ".data");
            for (const auto &way : ways) {
                std::cout << way << '\n';
                DependencyGraph dep_graph(data_graph);
                if (way != "baseline") {
                    auto time_st = std::chrono::system_clock::now();
                    dep_graph.decompose();
                    auto time_en = std::chrono::system_clock::now();
                    int node = 0;
                    for (const auto &e : dep_graph.sub_graphs) {
                        node += e.graph_data.size();
                    }
                    std::cout << dep_graph.sub_graphs.size() << ' ' << node << '\n';
                    std::cout << "build dep cost :" << time_cost(time_st, time_en) << '\n';
                }
                std::string parameters = "6-1.15";
                std::cout << parameters << '\n';
                std::string out_file_name = "../Results/" + way + "-" + data_set + "-" + parameters + "-L" + std::to_string(L) +".csv";
                std::ofstream of(out_file_name);
                for (int i = 0; i < 25; ++i) {
                    PatternGraph pattern_graph;
                    pattern_graph.loader("wiki-talk/" + parameters + "/" + std::to_string(i) + ".p");
                    if (way == "baseline") {
                        Baseline bs(data_graph, pattern_graph);
                        auto time_st = std::chrono::system_clock::now();
                        int num = bs.match2(L, S, T, K);
                        auto time_en = std::chrono::system_clock::now();
                        std::cout << i << "     " << time_cost(time_st, time_en) << "     " << num << "\n";
                        of << i << ',' << time_cost(time_st, time_en) << ',' << num << "\n";
                    } else {
                        std::vector<int> all_space(dep_graph.sub_graphs.size());
                        std::iota(all_space.begin(), all_space.end(), 0);
                        std::set<int> sp(all_space.begin(), all_space.end());
                        auto time_st = std::chrono::system_clock::now();
                        SimBalancer simbl(THREAD_SIZE, L, K, S, T, dep_graph, pattern_graph, {sp});
                        int num = simbl.launch();
                        auto time_en = std::chrono::system_clock::now();
                        std::cout << i << "     " << time_cost(time_st, time_en) << "     " << num << "\n";
                        of << i << ',' << time_cost(time_st, time_en) << ',' << num << "\n";
                    }
                }
                of.close();
            }
        }
    }
    return 0;
}
