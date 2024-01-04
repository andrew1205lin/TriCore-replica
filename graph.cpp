/*
 * Author: Andrew
 * Date: January 1, 2024
 * Description: 
 *   This file contains the definition of CSRGraph.
 * About to add a function so it could read mmio.
 *
 * Appendices:
 * 1. shout out to slimon writing most of the code.
 */
#include "graph.h"
#include "mmio.h"

CSRGraph::CSRGraph(const std::string &filename) {
    if (filename.substr(filename.find_last_of(".") + 1) == "txt") {
        buildFromTxtFile(filename);
    } else if (filename.substr(filename.find_last_of(".") + 1) == "mmio") {
        buildFromMmioFile(filename);
    } else {
        std::cerr << "Unsupported file type for: " << filename << '\n';
        exit(1);
    }
}

void CSRGraph::buildFromTxtFile(const std::string &filename) {
  std::ifstream file(filename);
  if (file.fail()) {
    fprintf(stderr, "\"%s\" does not exist!\n", filename.c_str());
    exit(1);
  }
  std::string line;
  std::unordered_map<int, std::vector<int>> adjacency_list;
  int cnt = 0;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    if (line.find("# Nodes:") != std::string::npos)
      sscanf(line.c_str(), "# Nodes: %d Edges: %d", &num_nodes, &num_edges);

    if (ss.str()[0] == '#')
      continue;
    int from, to;
    ss >> from >> to;
    adjacency_list[from].push_back(to);
    cnt++;
  }

  // num_nodes = adjacency_list.size();
  offsets.push_back(0);
  for (int i = 0; i < num_nodes; ++i) {
    // some nodes have no out edges
    if (adjacency_list.count(i)) {
      offsets.push_back(offsets.back());
      continue;
    }
    sort(adjacency_list[i].begin(), adjacency_list[i].end());
    for (int neighbor : adjacency_list[i])
      destinations.push_back(neighbor);
    offsets.push_back(destinations.size());
  }

  num_edges_processed = cnt;
  std::cout << "nodes: " << num_nodes << std::endl;
  std::cout << "edges: " << num_edges << std::endl;
  std::cout << "edges processed: " << num_edges_processed << std::endl;
  // assert(num_nodes == offsets.size() - 1);
  // assert(num_edges == destinations.size());

  // for (auto x : destinations) std::cout << x << ' '; std::cout <<
  // std::endl; for (auto x : offsets) std::cout << x << ' '; std::cout <<
  // std::endl;

  // for (int u = 0; u < num_nodes; ++u) {
  //     cout << u << ": ";
  //     for (int v = offsets[u]; v < offsets[u+1]; ++v)
  //         cout << v << ' ';
  //     cout << '\n';
  // }
}

void CSRGraph::buildFromMmioFile(const std::string &filename) {
  FILE *f;
  int ret_code;
  MM_typecode matcode;
  int M, N, nz; //rows, columns, entries. #vertex, #vertex, #edges in graph 
  int *I, *J;
  double *val;

  if ((f = fopen(filename.c_str(), "r")) == NULL) {
    fprintf(stderr, "\"%s\" does not exist!\n", filename.c_str());
    exit(1);
  }

  if (mm_read_banner(f, &matcode) != 0)
  {
    printf("Could not process Matrix Market banner.\n");
    exit(1);
  }

  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */
  if (mm_is_complex(matcode) && mm_is_matrix(matcode) && 
          mm_is_sparse(matcode) ) {
      printf("Sorry, this application does not support ");
      printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
      exit(1);
  }

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) !=0){
    printf("read mtx size error.");
    exit(1);
  }

  num_nodes = M;
  num_edges = nz;
    
  I = (int *) malloc(nz * sizeof(int));
  J = (int *) malloc(nz * sizeof(int));
  val = (double *) malloc(nz * sizeof(double));

  std::unordered_map<int, std::vector<int>> adjacency_list;
  int cnt = 0;

  for (int i=0; i<nz; i++)
  {
    if (fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]) != 3) {
      fprintf(stderr, "Error reading values from file.\n");
      exit(EXIT_FAILURE);
    }
    I[i]--;  /* adjust from 1-based to 0-based */
    J[i]--;
    adjacency_list[I[i]].push_back(J[i]);
    cnt++;
  }

  if (f !=stdin) fclose(f);

  offsets.push_back(0);
  for (int i = 0; i < num_nodes; ++i) {
    // some nodes have no out edges
    if (!adjacency_list.count(i)) {
      // std::cout << "node " << i << " has no friend." << std::endl;
      offsets.push_back(offsets.back());
      continue;
    }
    sort(adjacency_list[i].begin(), adjacency_list[i].end());
    for (int neighbor : adjacency_list[i])
      destinations.push_back(neighbor);
    offsets.push_back(destinations.size());
  }

  num_edges_processed = cnt;
  std::cout << "nodes: " << num_nodes << std::endl;
  std::cout << "edges: " << num_edges << std::endl;
  std::cout << "edges processed: " << num_edges_processed << std::endl;
}

void CSRGraph::saveToBinary(const std::string &filename) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Could not open file for writing: " << filename << '\n';
    return;
  }
  file.write(reinterpret_cast<const char *>(&num_nodes), sizeof(num_nodes));
  file.write(reinterpret_cast<const char *>(&num_edges), sizeof(num_edges));
  file.write(reinterpret_cast<const char *>(&num_edges_processed),
             sizeof(num_edges_processed));
  file.write(reinterpret_cast<const char *>(destinations.data()),
             destinations.size() * sizeof(int));
  file.write(reinterpret_cast<const char *>(offsets.data()),
             offsets.size() * sizeof(int));
}

void CSRGraph::checkIfContinuous(){
  //result: every node in amazon0302 has at least one edge.

  std::cout << "destination nodes: " << destinations.size() << std::endl;

  std::vector<int> nodeNoOut;
  int prev_off = -1,i=0;
  for(int &off:offsets){
    if(prev_off == off) {
      //std::cout << "node "<< i << " has no out."<< std::endl; 
      nodeNoOut.push_back(i); //already 1-index
    }
    prev_off = off;
    i++;
  }

  std::set<int> nodeNoIn;
  for (int i = 1; i <= num_nodes; ++i) {
    nodeNoIn.insert(i);
  }
  for (int num : destinations) {
    auto it = nodeNoIn.find(num+1); //back to 1-index
    if (it != nodeNoIn.end()) {
      nodeNoIn.erase(it);
    }
  }    

  std::set<int> intersectionSet;
  std::set_intersection(
    nodeNoIn.begin(), nodeNoIn.end(),
    nodeNoOut.begin(), nodeNoOut.end(),
    std::inserter(intersectionSet, intersectionSet.begin())
  );

  std::cout << "Intersection: ";
  for (int num : intersectionSet) {
    std::cout << num << " ";
  }
  std::cout << std::endl;

}

void CSRGraph::loadFromBinary(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Could not open file for reading: " << filename << '\n';
    return;
  }
  file.read(reinterpret_cast<char *>(&num_nodes), sizeof(num_nodes));
  file.read(reinterpret_cast<char *>(&num_edges), sizeof(num_edges));
  file.read(reinterpret_cast<char *>(&num_edges_processed),
            sizeof(num_edges_processed));
  destinations.resize(num_edges_processed);
  offsets.resize(num_nodes + 1);
  file.read(reinterpret_cast<char *>(destinations.data()),
            destinations.size() * sizeof(int));
  file.read(reinterpret_cast<char *>(offsets.data()),
            offsets.size() * sizeof(int));
  std::cout << "nodes: " << num_nodes << std::endl;
  std::cout << "edges: " << num_edges << std::endl;
  std::cout << "num_edges_processed: " << num_edges_processed << std::endl;
}

int main() {
  CSRGraph graph("amazon0302_adj.mmio");

  graph.checkIfContinuous();
  graph.saveToBinary("./graph_binary/amazon0302_adj.bin");

  CSRGraph loadedGraph;
  loadedGraph.loadFromBinary("./graph_binary/amazon0302_adj.bin");

  return 0;
}