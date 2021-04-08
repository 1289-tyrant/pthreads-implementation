#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define VERTICES 4
#define NTHREADS 2
#define MAX 100000

int graph[VERTICES][VERTICES];
int visited[VERTICES];
int no_of_visited = 0;
int dis[VERTICES][2];
int flag = 1;
pthread_mutex_t lock;

unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}


struct threadData{
    int current_node;
    int start;
};

int shortest_distance_node(){
    int dist = MAX;
    int vertex = -1;
    for(int i = 0; i < VERTICES; i++){
        if(dist > dis[i][0]){
            dist = dis[i][0];
            vertex = i;
        }
    }
    visited[vertex] = 1;
    return vertex;
}

void *update_distances(void *arg){
    struct threadData *temp = (struct threadData *)arg;
    int current_node = temp->current_node;
    int start = (temp->start)*NTHREADS, end = (temp->start + 1)*NTHREADS;
    for(int i = start; i < end; i++){
        if(graph[current_node][i] == 0)continue;
        int new_distance = dis[current_node][0] + graph[current_node][i];
        if(new_distance < dis[i][0]){
            dis[i][0] =  new_distance;
            dis[i][1] = current_node;
        }
    }
}

void dj(){
    printf("Select the starting vertex\n");
    int node;
    scanf("%d", &node);
    dis[node][1] = -1;
    dis[node][0] = 0;
    visited[node] = 1;
    while(no_of_visited < VERTICES){
        int current_node = shortest_distance_node();
        no_of_visited++;
        pthread_t thread[NTHREADS];
        for(int i = 0; i < VERTICES/NTHREADS; i++){
            struct threadData t;
            t.current_node = current_node;
            t.start = i;
            pthread_create(&thread[i], NULL, update_distances, &t);
        }
        for(int l = 0; l < VERTICES/NTHREADS; l++)
            pthread_join(thread[i], NULL);
    }
    for(int i = 0; i < VERTICES; i++){
        printf("distance: %d, parent: %d\n", dis[i][0], dis[i][1]);
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
    for(int i = 0; i < VERTICES; i++){
        visited[i] = 0;
        dis[i][0] = MAX;
        dis[i][1] = -1;
    }
    printf("\n");
    dj();
}