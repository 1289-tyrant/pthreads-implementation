/*
AS Algorithm
*/
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

#define VERTICES 7
#define NTHREADS 3
#define MAX 100000

int graph[VERTICES][VERTICES] = {
                                    {0,0,1,1,0,0,0},
                                    {0,0,0,0,1,0,0},
                                    {0,0,0,0,0,1,1},
                                    {1,0,0,0,1,0,0},
                                    {0,1,0,1,0,0,0},
                                    {0,0,1,0,0,0,0},
                                    {0,0,1,0,0,0,0}
                                };
int edges[VERTICES*VERTICES/2][2];
int size = 0;
int f[VERTICES];
int gf[VERTICES];
int visited[VERTICES];
int no_of_visited = 0;
int dis[VERTICES][2];
int flag = 1;
pthread_mutex_t lock[VERTICES];

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

int belongs(int u){
    if(f[f[u]] == f[u])return 1;
    else return 0;
}

void *conditionalStarHooking(void *arg){
    int *temp = (int *)arg;
    int start = (*temp)*NTHREADS, end = (*temp + 1)*NTHREADS;
    for(int i = start; i < end; i++){
        int u = edges[i][0], v = edges[i][1];
        if(belongs(u) && f[u] > f[v]){
            f[f[u]] = f[v];
            flag = 1;
        }
    }
}

void *unconditionalStarHooking(void *arg){
    int *temp = (int *)arg;
    int start = (*temp)*NTHREADS, end = (*temp + 1)*NTHREADS;
    for(int i = start; i < end; i++){
        int u = edges[i][0], v = edges[i][1];
        if(belongs(u) && f[u] != f[v]){
            f[f[u]] = f[v];
            flag = 1;
        }
    }
}

void *shortcutting1(void *arg){
    int *temp = (int *)arg;
    int start = (*temp)*NTHREADS, end = (*temp + 1)*NTHREADS;
    for(int i = start; i < end; i++){
        gf[i] = f[f[i]];
    }
}

void *shortcutting2(void *arg){
    int *temp = (int *)arg;
    int start = (*temp)*NTHREADS, end = (*temp+1)*NTHREADS;
    for(int i = start; i < end; i++){
        if(belongs(i) == 0){
            f[i] = gf[i];
            flag = 1;
        }
    }
}

void connectedComponents(){
    int i;
    for(i = 0; i < VERTICES; i++)
        f[i] = i;
    pthread_t thread1[NTHREADS], thread2[NTHREADS], thread3[NTHREADS], thread4[NTHREADS];
    int x = 0;
    do{
        flag = 0;
        for(i = 0; i < size/NTHREADS; i++)
            pthread_create(&thread1[i], NULL, conditionalStarHooking, &i);
        for(int l = 0; l < size/NTHREADS; l++)
            pthread_join(thread1[l], NULL);
        
        for(i = 0; i < size/NTHREADS; i++)
            pthread_create(&thread2[i], NULL, unconditionalStarHooking, &i);
        for(int l = 0; l < size/NTHREADS; l++)
            pthread_join(thread2[l], NULL);
        
        for(i = 0; i < VERTICES/NTHREADS; i++){
            pthread_create(&thread3[i], NULL, shortcutting1,&i);
        }
        for(int l = 0; l < VERTICES/NTHREADS; l++)
            pthread_join(thread3[l], NULL);
        
        for(i = 0; i < VERTICES/NTHREADS; i++){
            pthread_create(&thread4[i], NULL, shortcutting2,&i);
        }
        for(int l = 0; l < VERTICES/NTHREADS; l++)
            pthread_join(thread4[l], NULL);
    }while(flag);
    int no_of_components = -1;
    for(int i = 0; i < VERTICES; i++){
        printf("%d: %d ", i, f[i]);
    }
    printf("\n");
}

int main(){
    int i, j;
    srand(time_seed());
    for(i = 0; i < VERTICES; i++)
        pthread_mutex_init(&lock[i], NULL);
    for(i = 0; i < VERTICES; i++){ 
		for(j=0; j<VERTICES; j++){
            if(graph[i][j] != 0){
                edges[size][0] = i;
                edges[size][1] = j;
                size ++;
            }
		}	
	}
    printf("=======GRAPH========\n");
    for(i = 0; i < VERTICES; i++){
        for(j = 0; j < VERTICES; j++){
            printf("%d ", graph[i][j]);
        }
        printf("\n");
    }
    connectedComponents();
}