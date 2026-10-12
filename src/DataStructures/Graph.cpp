// ============================================================================
//  Graph.cpp
// ============================================================================
#include "Graph.h"
#include "Queue.h"
#include <iostream>

Graph::Graph(int initialCapacity)
    : vertices(nullptr), vertexCount(0), capacity(initialCapacity > 0 ? initialCapacity : 8)
{
    vertices = new Vertex[capacity];
}

Graph::~Graph()
{
    delete[] vertices;         // moi Vertex tu huy DoublyLinkedList<int> cua no
}

void Graph::grow()
{
    int newCapacity = capacity * 2;
    Vertex* bigger = new Vertex[newCapacity];
    for (int i = 0; i < vertexCount; i++) bigger[i] = vertices[i];   // deep copy (DLL::operator=)
    delete[] vertices;
    vertices = bigger;
    capacity = newCapacity;
}

int Graph::findVertex(const std::string& name) const
{
    for (int i = 0; i < vertexCount; i++)
        if (vertices[i].name == name) return i;
    return -1;
}

int Graph::addVertex(const std::string& name)
{
    if (name.empty() || findVertex(name) != -1) return -1;
    if (vertexCount == capacity) grow();
    vertices[vertexCount].name = name;
    vertexCount++;
    return vertexCount - 1;
}

bool Graph::addEdge(const std::string& a, const std::string& b)
{
    int ia = findVertex(a);
    int ib = findVertex(b);
    if (ia == -1 || ib == -1 || ia == ib) return false;
    if (vertices[ia].neighbors.contains(ib)) return false;   // canh da ton tai
    vertices[ia].neighbors.addLast(ib);                      // do thi vo huong:
    vertices[ib].neighbors.addLast(ia);                      // them ca 2 chieu
    return true;
}

// BFS: duyet theo "lop" - tham tat ca dinh cach start 1 buoc, roi 2 buoc, ...
// Dung Queue (FIFO). Do phuc tap O(V + E).
void Graph::BFS(const std::string& startName) const
{
    int start = findVertex(startName);
    if (start == -1)
    {
        std::cout << "Khong tim thay khu vuc \"" << startName << "\"\n";
        return;
    }

    bool* visited = new bool[vertexCount];
    int* distance = new int[vertexCount];
    for (int i = 0; i < vertexCount; i++)
    {
        visited[i] = false;
        distance[i] = 0;
    }

    Queue<int> waiting;
    visited[start] = true;
    waiting.enqueue(start);

    std::cout << "BFS tu " << vertices[start].name << ":\n";
    int order = 1;
    int current = 0;
    while (waiting.dequeue(current))
    {
        std::cout << "  " << order++ << ". " << vertices[current].name
                  << "  (cach " << vertices[start].name << " " << distance[current] << " buoc)\n";

        for (Node<int>* n = vertices[current].neighbors.getHead(); n != nullptr; n = n->next)
        {
            int neighbor = n->data;
            if (!visited[neighbor])
            {
                visited[neighbor] = true;                    // danh dau NGAY khi cho vao hang doi
                distance[neighbor] = distance[current] + 1;
                waiting.enqueue(neighbor);
            }
        }
    }

    delete[] visited;
    delete[] distance;
}

// DFS de quy: di sau nhat co the theo mot nhanh, het duong thi quay lui.
void Graph::dfsVisit(int v, bool* visited, int depth, int& order) const
{
    visited[v] = true;
    std::cout << "  " << order++ << ". ";
    for (int i = 0; i < depth; i++) std::cout << "  ";       // thut le the hien do sau de quy
    std::cout << vertices[v].name << "\n";

    for (Node<int>* n = vertices[v].neighbors.getHead(); n != nullptr; n = n->next)
        if (!visited[n->data]) dfsVisit(n->data, visited, depth + 1, order);
}

void Graph::DFS(const std::string& startName) const
{
    int start = findVertex(startName);
    if (start == -1)
    {
        std::cout << "Khong tim thay khu vuc \"" << startName << "\"\n";
        return;
    }

    bool* visited = new bool[vertexCount];
    for (int i = 0; i < vertexCount; i++) visited[i] = false;

    std::cout << "DFS tu " << vertices[start].name << " (thut le = do sau de quy):\n";
    int order = 1;
    dfsVisit(start, visited, 0, order);

    delete[] visited;
}

void Graph::display() const
{
    if (vertexCount == 0)
    {
        std::cout << "(do thi rong)\n";
        return;
    }
    std::cout << "Do thi co " << vertexCount << " khu vuc. Danh sach ke:\n";
    for (int i = 0; i < vertexCount; i++)
    {
        std::cout << "  " << vertices[i].name << " -> ";
        if (vertices[i].neighbors.isEmpty()) std::cout << "(chua noi voi khu vuc nao)";
        for (Node<int>* n = vertices[i].neighbors.getHead(); n != nullptr; n = n->next)
        {
            std::cout << vertices[n->data].name;
            if (n->next != nullptr) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

int Graph::getVertexCount() const { return vertexCount; }
