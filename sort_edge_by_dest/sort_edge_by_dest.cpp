#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

struct Edge {
    int fromNodeId;
    int toNodeId;
};

bool compareEdges(const Edge& edge1, const Edge& edge2) {
    return edge1.toNodeId < edge2.toNodeId;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file!" << std::endl;
        return 1;
    }

    std::vector<Edge> edges;
    int fromNodeId, toNodeId;

    while (inputFile >> fromNodeId >> toNodeId) {
        edges.push_back({fromNodeId, toNodeId});
    }

    inputFile.close();

    std::sort(edges.begin(), edges.end(), compareEdges);

    std::ofstream outputFile(argv[2]);

    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file!" << std::endl;
        return 1;
    }

    std::cout << "num edges: " << edges.size() << std::endl;
    for (const Edge& edge : edges) {
        outputFile << edge.fromNodeId << " " << edge.toNodeId << std::endl;
    }

    outputFile.close();

    std::cout << "Sorting and writing completed successfully." << std::endl;

    return 0;
}