#include <iostream>
#include <queue>
#include <vector>

int main() {
    // Max-heap (default behavior)
    std::priority_queue<int> maxHeap;

    maxHeap.push(10);
    maxHeap.push(5);
    maxHeap.push(20);

    // Print and pop elements
    std::cout << "Max-Heap elements (in order): ";
    while (!maxHeap.empty()) {
        std::cout << maxHeap.top() << " "; // Get largest element
        maxHeap.pop(); // Remove it
    }
    std::cout << std::endl;

    // Min-heap (using greater<> comparator)
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;

    minHeap.push(10);
    minHeap.push(5);
    minHeap.push(20);

    // Print and pop elements
    std::cout << "Min-Heap elements (in order): ";
    while (!minHeap.empty()) {
        std::cout << minHeap.top() << " "; // Get smallest element
        minHeap.pop(); // Remove it
    }
    std::cout << std::endl;

    return 0;
}