#ifndef	COMM_HEADER
#define	COMM_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
//--------------------------
//typedef 	unsigned long int 	index_t;
//typedef		unsigned int		vertex_t;

typedef 	uint64_t 	index_t;
typedef		uint32_t	vertex_t;
//--------------------------------

inline off_t fsize(const char *filename) {
	struct stat st;
	if (stat(filename, &st) == 0){
		return st.st_size;
	}
	return -1;
}
typedef struct Edge{
	vertex_t A;
	vertex_t B;
}EDGE;


#endif
