/*
 * Author: Andrew
 * Date: January 1, 2024
 * Description: 
 *   This file contains the definition of CSRGraph
 *
 * Appendices:
 * 1. shout out to slimon writing most of the code.
 * 2. num_edges_processed is to verify #edges.
 */
#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// CSR graph representation
class CSRGraph {
public:
  CSRGraph() = default;
  CSRGraph(const std::string &filename);
  void buildFromTxtFile(const std::string &filename);
  void buildFromMmioFile(const std::string &filename);
  void saveToBinary(const std::string &filename);
  void loadFromBinary(const std::string &filename);
  bool operator==(const CSRGraph &rhs) const {
    return num_nodes == rhs.num_nodes && num_edges == rhs.num_edges &&
           destinations == rhs.destinations && offsets == rhs.offsets;
  }

  int num_nodes;
  int num_edges;
  int num_edges_processed;
  std::vector<int> destinations;
  std::vector<int> offsets;
};