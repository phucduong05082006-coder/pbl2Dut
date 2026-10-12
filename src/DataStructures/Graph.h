// ============================================================================
//  Graph.h - Do thi vo huong, danh sach ke bang DoublyLinkedList<int>
//  Cai dat: Graph.cpp
// ============================================================================
#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include "DoublyLinkedList.h"

struct Vertex
{
    std::string name;                              // ten khu vuc
    DoublyLinkedList<int> neighbors;               // danh sach chi so cac dinh ke
};

class Graph
{
private:
    Vertex* vertices;                              // mang dong cac dinh
    int vertexCount;
    int capacity;

    void grow();                                   // nhan doi capacity khi day
    void dfsVisit(int v, bool* visited, int depth, int& order) const;

public:
    explicit Graph(int initialCapacity = 8);
    ~Graph();

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    int addVertex(const std::string& name);        // tra ve chi so, -1 neu trung ten
    int findVertex(const std::string& name) const; // -1 neu khong co
    bool addEdge(const std::string& a, const std::string& b);   // canh 2 chieu

    void BFS(const std::string& start) const;      // dung Queue<int>
    void DFS(const std::string& start) const;      // de quy

    void display() const;
    int getVertexCount() const;
};

#endif
