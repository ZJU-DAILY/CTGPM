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

    std::vector<std::string> ways = {"opt", "baseline"}; // opt->DTmatch, baseline->BCTmatch
    std::vector<std::string> data_sets = {"highschool", "mathoverflow", "superuser", "wiki-talk"};
    for (int L = 1; L <= 1; ++L) {
        for (const auto &data_set : data_sets) {
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

                std::vector<std::pair<std::string, std::string>> para = {
                        {"3", "1.15"}, {"4", "1.15"}, {"5", "1.15"},
                        {"6", "1.15"},
                        {"6", "1.05"}, {"6", "1.1"}, {"6", "1.2"}, {"6", "1.25"},
                        {"7", "1.15"},
                        {"8", "1.15"},
                };
                for (const auto &[p1, p2] : para) {
                    std::string parameters = p1 + "-" + p2;
                    std::cout << parameters << '\n';
                    std::string out_file_name = "../Results/" + way + "-" + data_set + "-" + parameters + "-L" + std::to_string(L) +".csv";
                    std::ofstream of(out_file_name);
                    for (int i = 0; i < 25; ++i) {
                        PatternGraph pattern_graph;
                        pattern_graph.loader(data_set + "/" + parameters + "/" + std::to_string(i) + ".p");
                        if (way == "baseline") {
                            Baseline bs(data_graph, pattern_graph);
                            auto time_st = std::chrono::system_clock::now();
                            int num = bs.match2(L, S, T, K);
                            auto time_en = std::chrono::system_clock::now();
                            std::cout << i << "     " << time_cost(time_st, time_en) << "     " << num << "\n";
                            of << i << ',' << time_cost(time_st, time_en) << ',' << num << "\n";
                        } else {
                            auto time_st = std::chrono::system_clock::now();
                            SimRunner simr(L, S, T, dep_graph, pattern_graph, {});
                            int num = simr();
                            auto time_en = std::chrono::system_clock::now();
                            std::cout << i << "     " << time_cost(time_st, time_en) << "     " << num << "\n";
                            of << i << ',' << time_cost(time_st, time_en) << ',' << num << "\n";
                        }
                    }
                    of.close();
                }
            }
        }
    }
    return 0;
}
