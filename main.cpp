#include <iostream>
using namespace std;

#include "DataStructures/DoublyLinkedList.h"
#include "DataStructures/HashTable.h"
#include "DataStructures/BinarySearchTree.h"
#include "DataStructures/Queue.h"
#include "DataStructures/Stack.h"
#include "DataStructures/PriorityQueue.h"

int main() {
    DoublyLinkedList<int> movieIds;
    movieIds.pushBack(101);
    movieIds.pushBack(102);
    cout << "DoublyLinkedList size: " << movieIds.size() << endl;

    HashTable<int, string> movies;
    movies.insert(101, "Example movie");
    cout << "HashTable lookup: " << *movies.find(101) << endl;

    BinarySearchTree<int> roomIds;
    roomIds.insert(20);
    roomIds.insert(10);
    roomIds.insert(30);
    cout << "BinarySearchTree contains room 10: "
         << (roomIds.contains(10) ? "yes" : "no") << endl;

    Queue<int> customerQueue;
    customerQueue.enqueue(1);
    cout << "Queue next customer: " << customerQueue.dequeue() << endl;

    Stack<string> operationHistory;
    operationHistory.push("Add movie");
    cout << "Stack last operation: " << operationHistory.pop() << endl;

    PriorityQueue<int> maintenancePriorities;
    maintenancePriorities.push(2);
    maintenancePriorities.push(5);
    cout << "PriorityQueue highest priority: "
         << maintenancePriorities.pop() << endl;

    return 0;
}
