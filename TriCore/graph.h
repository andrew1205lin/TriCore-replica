#ifndef	GRAPH_H
#define	GRAPH_H
#include "comm.h"
#include <string>

class graph{
	
	//variable
public:
	vertex_t 	vert_count;
	index_t		edge_count;
	vertex_t	*adj_list;
	index_t		*beg_pos;
	EDGE		*edge;
	

	//after partition
	
	index_t		*count;	


	//gpu data
//	GPU_data *gdata;

	//constructor
	graph() {};
	graph(	std::string filename);//,

	void validation();
	void scan();
	

};

#endif
