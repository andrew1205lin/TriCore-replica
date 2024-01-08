//scan.cu
#include "comm.h"
#include "cuUtil.cu"
#include "graph.h"
#include "wtime.h"
#include "gputimer.h" //
#include "iostream"
#define max_thd 256 
#define max_block 256 

using namespace std;


__global__ void warp_binary_kernel
(	
	vertex_t*	adj_list,
	index_t*	begin,
	EDGE*		edge,
	index_t		Ns,
	index_t		Ne,
	index_t*	count
)
{
	//phase 1, partition
	index_t tid = (threadIdx.x + blockIdx.x * blockDim.x)/32 + Ns;
	index_t mycount=0;
	__shared__ index_t local[max_thd];

	int i = threadIdx.x%32;
	int p = threadIdx.x/32;

	while(tid<Ne){
		vertex_t A = edge[tid].A;
		vertex_t B = edge[tid].B;
		index_t m = begin[A+1]-begin[A];//degree[A];
		index_t n = begin[B+1]-begin[B];//degree[B];

		index_t temp;	
		if(m<n){
			temp = A;
			A = B;
			B = temp;
			temp = m;
			m = n;
			n = temp;
		}

		vertex_t* a = &(adj_list[begin[A]]);
		vertex_t* b = &(adj_list[begin[B]]);
		
	//initial cache
		local[p*32+i]=a[i*m/32];	
		__syncthreads();
			
	//search
		int j=i;
		while(j<n){
			vertex_t X = b[j];
			vertex_t Y;
			//phase 1: cache
			int bot = 0;
			int top = 32;
			int r;
			while(top>bot+1){
				r = (top+bot)/2;
				Y = local[p*32+r];
				if(X==Y){
					mycount++;
					bot = top + 32;
				}
				if(X<Y){
					top = r;
				}
				if(X>Y){
					bot = r;
				}
			}
			//phase 2
			bot = bot*m/32;
			top = top*m/32 -1;
			while(top>=bot){
				r = (top+bot)/2;
				Y = a[r];
				if(X==Y){
					mycount++;
				}
				if(X<=Y){
					top = r-1;
				}
				if(X>=Y){
					bot = r+1;
				}
			}
			j += 32;
		
		}
		tid += blockDim.x*gridDim.x/32;
		__syncthreads();
	}

	__syncthreads();
	//reduce
	local[threadIdx.x] = mycount;
	__syncthreads();
	if(threadIdx.x==0){
		index_t val=0;
		for(int i=0; i<blockDim.x; i++){
			val+= local[i];
		}
		count[blockIdx.x]=val;
	}
	__syncthreads();

}



__global__ void reduce_kernel2(index_t* count)
{
	index_t val = 0;
	for(int i=0; i<max_block; i++){
		val += count[i];
	}
	count[0] = val;
}

//---------------------------------------- cpu function--------------------
//------------------------------------------------------------------

void graph::scan(){


	vertex_t*	dev_adj;
	index_t*	dev_begin;
	EDGE*		dev_edge;
	index_t*	dev_count;
	GpuTimer gpu_timer; //

	H_ERR(cudaMalloc(&dev_adj, edge_count*sizeof(vertex_t)) );
	H_ERR(cudaMalloc(&dev_begin,  (vert_count+1)*sizeof(index_t)) );
	H_ERR(cudaMalloc(&dev_edge,  (edge_count)*sizeof(EDGE)) );
	H_ERR(cudaMalloc(&dev_count,    max_block*sizeof(index_t)) );

		
	H_ERR(cudaMemcpy(dev_adj,    adj_list, 	edge_count*sizeof(vertex_t), cudaMemcpyHostToDevice) );
	H_ERR(cudaMemcpy(dev_begin,  beg_pos,  	(vert_count+1)*sizeof(index_t),  cudaMemcpyHostToDevice) );
	H_ERR(cudaMemcpy(dev_edge,   edge, 	edge_count*sizeof(EDGE), cudaMemcpyHostToDevice) );



	double time1=wtime(); //?
	//gpu_timer.Start(); //
	H_ERR(cudaDeviceSynchronize() );
	warp_binary_kernel<<<max_block,max_thd>>>
	(	
		dev_adj,
		dev_begin,
		dev_edge,
		0,
		edge_count,
		dev_count
	);
	H_ERR(cudaDeviceSynchronize() );
	reduce_kernel2 <<<1,1>>>(dev_count);
	H_ERR(cudaDeviceSynchronize() );
	
	H_ERR(cudaMemcpy(count, dev_count, sizeof(index_t), cudaMemcpyDeviceToHost));
		

	double time2 = wtime();
	//gpu_timer.Stop(); //
	cout<<"GPU processing wtime = "<<time2-time1<<endl;
	cout<<"GPU processing gputime = "<<gpu_timer.Elapsed()<<endl;//
	cout<<"GPU triangle count = "<<count[0]<<endl;


	H_ERR(cudaFree(dev_adj) );
	H_ERR(cudaFree(dev_edge) );
	H_ERR(cudaFree(dev_begin) );
	
	H_ERR(cudaFree(dev_count) );
	return;	
}


