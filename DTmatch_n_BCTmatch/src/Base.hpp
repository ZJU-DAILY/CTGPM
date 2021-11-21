/*
 * Copyright (c) 2021 by Contributors
 * \file Base.hpp
 * \date 2021-10
 * \author Xinwei Cai
 */
#pragma once

#include "Core.h"

[[maybe_unused]] const int I32_MAX = std::numeric_limits<int>::max();
[[maybe_unused]] const int I32_MIN = std::numeric_limits<int>::min();
[[maybe_unused]] const int64_t I64_MAX = std::numeric_limits<int64_t>::max();
[[maybe_unused]] const int64_t I64_MIN = std::numeric_limits<int64_t>::min();

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
    int seq;
    Edge() = default;
    Edge(int _src, int _des, int _stime, int _etime, int _seq = 0):
    source_seq(_src), destination_seq(_des), start_time(_stime), end_time(_etime), seq(_seq){}
};

struct DataNode: Node<int> {
    std::vector<int> rev_edge;
    DataNode() = default;
    DataNode(int _seq, std::string _id, std::string _lab, std::vector<int> _adj):
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

template<class T>
void sort_unique(std::vector<T> &vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

template<class T, class Fun>
void sort_unique(std::vector<T> &vec, Fun &&fun) {
    std::sort(vec.begin(), vec.end(), fun);
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

template<class T>
std::ostream& operator <<(std::ostream &out, std::vector<T> &vec) {
    out << vec.size() << " \n"[vec.empty()];
    for (int i = 0; i < vec.size(); ++i) {
        out << vec[i] << " \n"[i + 1 == (int) vec.size()];
    }
    return out;
}

template<class T>
std::istream& operator >>(std::istream &in, std::vector<T> &vec) {
    int sz;
    in >> sz;
    vec.resize(sz);
    for (auto &e : vec) in >> e;
    return in;
}

template<class T>
std::string to_str(T para) {
    if constexpr(std::is_same<typename std::decay<T>::type, std::string>::value) {
        return para;
    } else {
        return std::to_string(para);
    }
}

template<class T, class ...Args>
std::string to_str(T para, Args &&...args) {
    return to_str(para) + "-" + to_str(args...);
}

/*
template<class T>
std::string to_str(T para) {
    return std::to_string(para);
}
template<>
std::string to_str(std::string para) {
    return para;
}
template<class T, class ...Args>
std::string to_str(T head, Args &&...args) {
    return to_str(head) + "-" + to_str(args...);
}*/
