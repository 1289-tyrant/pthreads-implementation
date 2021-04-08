#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define VERTICES 4
#define NTHREADS 2
#define MAX 100000

int graph[VERTICES][VERTICES];
int distance[VERTICES][VERTICES];

unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}

struct threadData{
    int src_node;
    int inter_node;
    int start;
};


void *update_distances(void *arg){
    struct threadData *temp = (struct threadData *)arg;
    int start = temp->start*NTHREADS, end = (temp->start + 1)*NTHREADS;
    int i = temp->src_node, k = temp->inter_node;
    for(int j = start; j < end; j++){
        if(distance[i][k] + distance[k][j] < distance[i][j]){
            distance[i][j] = distance[i][k] + distance[k][j];
        }
    }
}

void floyd(){
    for(int k = 0; k < VERTICES; k++){
        for(int i = 0; i < VERTICES; i++){
            pthread_t thread[NTHREADS];
            for(int j = 0; j < VERTICES/NTHREADS; j++){
                struct threadData t;
                t.src_node = i;
                t.inter_node = k;
                t.start = j;
                pthread_create(&thread[j], NULL, update_distances, &t);
            }
            for(int l = 0; l < VERTICES/NTHREADS; l++)
                pthread_join(thread[l], NULL);
        }
    }   
    printf("=======distances========\n");
    for(int i = 0;  i < VERTICES; i++){
        for(int j = 0; j < VERTICES; j++){
            printf("%d ", distance[i][j]);
        }
        printf("\n");
    }
}
   

int main(){
    int i, j;
    srand(time_seed());
    for(i=0; i<VERTICES; i++){ 
		for(j=0; j<VERTICES; j++){
			if(i == j){
				graph[i][i] = 0;
				continue;
			}
			int r = rand() % 100;			
			graph[i][j] = r;
			graph[j][i] = r;
		}	
	}
    printf("=======GRAPH========\n");
    for(i = 0; i < VERTICES; i++){
        for(j = 0; j < VERTICES; j++){
            printf("%d ", graph[i][j]);
        }
        printf("\n");
    }
    for(i = 0; i < VERTICES; i++){
        for(j = 0; j < VERTICES; j++){
            distance[i][j] = graph[i][j];
        }
    }
    floyd();
}