#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

#define NUM_THREADS 5;

typedef struct data
{
	int size;
	int* distances;
	int* selected_vertices;
	int* vertices;
	int** graph;
	int closest_vertex;
	int no_of_threads;
} Data;

unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}


typedef struct threadData{
	struct data* data_pointer;
	int thread_id;
} ThreadData;

int closest_vertex(int distances[], int selected_vertices[], int size);
int in_array(int selected_vertices[], int size, int ele);
int min(int a, int b);
void *updateDistances(void* d);

int min(int a, int b) { return (a < b) ? a : b; }

int closest_vertex(int distances[], int selected_vertices[], int size){
	int i;
	int vertex = -1;
	int min_distance = INT_MAX;
	for(i = 0; i < size; i++){
		if(in_array(selected_vertices,size,i) == 0){
			int len = distances[i];
			if(len != 0 && len < min_distance){
				vertex = i;
				min_distance = distances[i];
			}				
		}
	}
	return vertex;
}

void *updateDistances(void* d){  
	ThreadData* threadDatax = (struct threadData *)d;
	Data* data = threadDatax->data_pointer;	
	int i;
	int start,end;
	int offset = data->size/ data->no_of_threads;
	start = threadDatax->thread_id * offset;
	end = (threadDatax->thread_id + 1) * offset - 1;
	for(i = start; i <= end; i++){
		if(in_array(data->selected_vertices,data->size,i) == 0){
			int v = data->closest_vertex;
			if(data->graph[i][v] != 0 && data->distances[i] > data->graph[i][v]){
				data->distances[i] = data->graph[i][v];	
				data->vertices[i] = v;
			}
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	printf("This program computes the minimum spanning tree of a weighted dense graph using pthreads.\n");
	if(argc != 3){
		printf("Usage: ./threads <no_of_nodes> <no_of_threads>\n");
		exit(0);
	}
	int size = atoi(argv[1]);
	int no_of_threads = atoi(argv[2]);
	

	int **mst;
	int i,j;
	Data data;
	ThreadData threadData[no_of_threads];
	pthread_t threads[no_of_threads];	
	struct timeval start_time, end_time;		
	
	data.size = size;
	data.no_of_threads = no_of_threads;
	data.distances = malloc(size * sizeof(int));
	data.vertices = malloc(size * sizeof(int));
	data.selected_vertices = malloc(size * sizeof(int));
	data.graph = (int**) malloc(size*sizeof(int*));
	mst = (int**) malloc(size*sizeof(int*));
	
	for(i = 0; i < size; i++){
		data.graph[i] = (int*) malloc(size*sizeof(int));
		mst[i] = (int*) malloc(size*sizeof(int));
	}

	for(i = 0; i < no_of_threads; i++){
		threadData[i].data_pointer = (struct data *)&data;
		threadData[i].thread_id = i;
	}
		
	for(i = 0; i < size; i++)
		for(j = 0; j < size; j++)
			mst[i][j] = 0;

	srand(time_seed());
	
	for(i=0; i<size; i++){ 
		for(j=0; j<size; j++){
			if(i == j){
				data.graph[i][i] = 0;
				continue;
			}
			int r = rand() % 100;			
			data.graph[i][j] = r;
			data.graph[j][i] = r;
		}		
	}
	for(i = 0; i < size; i++)
		data.selected_vertices[i] = -1;
	int no_of_selected_vertices = 1;
    printf("Enter the source vertex\n");
	int first_vertex;
    scanf("%d", &first_vertex);
	data.selected_vertices[0] = first_vertex;
	for(i = 0; i < size; i++){
		data.distances[i] = data.graph[first_vertex][i];
		data.vertices[i] = first_vertex;
	}
	
	while(no_of_selected_vertices <= size){
		int v = closest_vertex(data.distances,data.selected_vertices,size);
		if(v == -1)
			break;
		data.closest_vertex = v;
		data.selected_vertices[no_of_selected_vertices] = data.closest_vertex;
		no_of_selected_vertices++;
		mst[data.closest_vertex][data.vertices[data.closest_vertex]] = 1;		
		int rc;
		int t;
		for(t=0; t<no_of_threads; t++){
			rc = pthread_create(&threads[t], NULL, updateDistances, (void*) &threadData[t]);
			if (rc){
				 printf("ERROR; return code from pthread_create() is %d\n", rc);
			 	exit(-1);
			}
			pthread_join(threads[t],NULL);			
		}
	}
	int mst_length = 0;
	for(i = 0; i < size; i++){
		for(j = 0; j < size; j++){			
			if(mst[i][j] == 1){
				mst_length += data.graph[i][j];
			}
		}		
	}
	printf("MST length: %d\n",mst_length);
}	

int in_array(int selected_vertices[], int size, int ele){
	int i = 0;
	for(i = 0; i < size ; i++){
		if(selected_vertices[i] == ele){
			return 1;
		}			
	}
	return 0;
}	

