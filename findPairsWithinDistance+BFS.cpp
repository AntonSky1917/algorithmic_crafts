#include <iostream>
#include <vector>
#include <queue>

class Graph {
public:
    Graph(int numVertices);
    void addEdge(int src, int dest);
    void findPairsWithinDistance(int distance);

private:
    std::vector<std::vector<int>> adjacencyList;
    int numVertices;

    void BFS(int startVertex, int distance, std::vector<bool>& visited);
};

Graph::Graph(int numVertices) : numVertices(numVertices) {
    adjacencyList.resize(numVertices);
}

void Graph::addEdge(int src, int dest) {
    adjacencyList[src].push_back(dest);
    adjacencyList[dest].push_back(src);
}

void Graph::findPairsWithinDistance(int distance) {
    for (int i = 0; i < numVertices; i++) {
        std::vector<bool> visited(numVertices, false);
        BFS(i, distance, visited);
    }
}

void Graph::BFS(int startVertex, int distance, std::vector<bool>& visited) {
    std::queue<int> queue;
    std::vector<int> levels(numVertices, -1);

    visited[startVertex] = true;
    levels[startVertex] = 0;
    queue.push(startVertex);

    while (!queue.empty()) {
        int currentVertex = queue.front();
        queue.pop();

        for (int neighbor : adjacencyList[currentVertex]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                levels[neighbor] = levels[currentVertex] + 1;
                queue.push(neighbor);

                if (levels[neighbor] <= distance) {
                    std::cout << "Pair: (" << startVertex << ", " << neighbor << "), Distance: " << levels[neighbor] << std::endl;
                }
            }
        }
    }
}

int main() {
    int numVertices = 6;
    Graph graph(numVertices);

    graph.addEdge(0, 1);
    graph.addEdge(0, 2);
    graph.addEdge(1, 3);
    graph.addEdge(2, 4);
    graph.addEdge(3, 4);
    graph.addEdge(3, 5);
    graph.addEdge(4, 5);

    int maxDistance = 3;
    graph.findPairsWithinDistance(maxDistance);

    return 0;
}
