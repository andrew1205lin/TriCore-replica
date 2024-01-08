#include "graph.h"
#include "comm.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <omp.h>

#include "graph.h"
#define FILE_NOT_EXIST	1
#define FILE_EXIST	0
#define P	32
using namespace std;

graph::graph(
	string jsonfile)//,
{
	cout<<"read from folder "<<jsonfile<<endl;
	
	string s_begin = jsonfile+"/begin.bin";
	string s_adj = jsonfile+"/adjacent.bin";
	string s_edge = jsonfile+"/edge";

	char* begin_file = const_cast<char*>(s_begin.c_str());
	char* adj_file = const_cast<char*>(s_adj.c_str());
	char* edge_file = const_cast<char*>(s_edge.c_str());

	vert_count = fsize(begin_file)/sizeof(index_t) - 1;
	edge_count = fsize(adj_file)/sizeof(vertex_t);

	cout<<"vert:"<< vert_count<<"  edge: "<<edge_count<<endl;


	FILE *pFile= fopen(adj_file,"rb");
	adj_list = (vertex_t *)malloc(fsize(adj_file));
	fread(adj_list,sizeof(vertex_t),edge_count,pFile);
	fclose(pFile);



	FILE *pFile3 = fopen(begin_file,"rb");
	beg_pos = (index_t *)malloc(fsize(begin_file));
	fread(beg_pos,sizeof(index_t),vert_count+1,pFile3);
	fclose(pFile3);

	FILE *pFile2 = fopen(edge_file,"rb");
	edge = (EDGE *)malloc(fsize(edge_file));
	fread(edge,sizeof(EDGE),edge_count,pFile2);
	fclose(pFile3);


	count = (index_t *)malloc(256*256*sizeof(index_t));
}


void graph::validation(){
	index_t mycount=0;

#pragma omp parallel for num_threads(56) reduction(+:mycount) schedule(dynamic,1024)
	for(index_t i=0; i<vert_count; i++){
		//cout << A << endl;
		for(index_t j = beg_pos[i]; j<beg_pos[i+1]; j++){
			vertex_t A=i;
			vertex_t B=adj_list[j];

			index_t m=beg_pos[A+1]-beg_pos[A];
			index_t n=beg_pos[B+1]-beg_pos[B];

	//cout<<"edge: "<<i<<" "<<U<<"-"<<V<<" ";
	//cout<<"degree: "<<m<<" "<<n<<endl;

			vertex_t *a = &adj_list[beg_pos[A]];
			vertex_t *b = &adj_list[beg_pos[B]];

			vertex_t u1=0;
			vertex_t v1=0;
			while(u1<m && v1<n){
				vertex_t x=a[u1];
				vertex_t y=b[v1];
				if(x<y){
					u1++;
				}
				else if(x>y){
					v1++;
				}
				else if(x==y){
					u1++;
					v1++;
					mycount++;
				}
			}
		}
	}
	cout<<"merge version tc = "<<mycount<<endl;
}


