#ifndef RDS2_BASIC_HPP
#define RDS2_BASIC_HPP

#include <string>
#include <utility>
#include <vector>
#include <utility>

template <class T>
struct Node {
    int seq{};
    std::string identity;
    std::string label;
    std::vector<T> adj_edge;
    Node() = default;
    Node(int _seq, std::string _id, std::string _lab, std::vector<T> _adj):
            seq(_seq), identity(std::move(_id)), label(std::move(_lab)), adj_edge(std::move(_adj)) {}
};

struct PatternNode: Node<int> {
    std::vector<int> rev_edge;
    PatternNode() = default;
    PatternNode(int _seq, std::string _id, std::string _lab, std::vector<int> _adj, std::vector<int> _rev):
            Node(_seq, std::move(_id), std::move(_lab), std::move(_adj)), rev_edge(std::move(_rev)) {}
};

struct Edge {
    int source_seq;
    int destination_seq;
    int start_time;
    int end_time;
    Edge() = default;
    Edge(int _src, int _des, int _stime, int _etime):
            source_seq(_src), destination_seq(_des), start_time(_stime), end_time(_etime){}
};

struct DataNode: Node<Edge> {
    DataNode() = default;
    DataNode(int _seq, std::string _id, std::string _lab, std::vector<Edge> _adj):
            Node(_seq, std::move(_id), std::move(_lab), std::move(_adj)) {}
};

struct Path: Edge {
    int length;
    Path() = default;
    Path(int _src, int _des, int _stime, int _etime, int _len):
            Edge(_src, _des, _stime, _etime), length(_len) {}
};

struct MatchNode: Node<Path> {
    MatchNode() = default;
    MatchNode(int _seq, std::string _id, std::string _lab, std::vector<Path> _adj):
            Node(_seq, std::move(_id), std::move(_lab), std::move(_adj)) {}
};

#endif //RDS2_BASIC_HPP
