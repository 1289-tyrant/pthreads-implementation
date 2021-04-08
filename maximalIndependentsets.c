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

#define VERTICES 10
#define NTHREADS 5

int graph[VERTICES][VERTICES];
int flag = 1;
pthread_mutex_t lock;

unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}

struct Independent_sets{
    int independent_sets[1024][VERTICES+1];
    int size;
}independent_sets;

struct TempSolutionsSet{
    int tempSolutionsSet[VERTICES+1];
    int size;
    int k;
    int vertex;
};

struct Maximal_Independent_sets{
    int max_independent_sets[1024][VERTICES+1];
    int size;
}maximalindependentsets;

void insert(struct TempSolutionsSet temp){
    int size = independent_sets.size;
    independent_sets.independent_sets[size][0] = 0;
    for(int i = 0; i < temp.size; i++){
        independent_sets.independent_sets[size][i+1] = temp.tempSolutionsSet[i];
        independent_sets.independent_sets[size][0]++;
    }
    independent_sets.size = size + 1;
}

void *isSafeForIndependentSet(void * arg){
    if(flag == 0)pthread_exit(NULL);
    struct TempSolutionsSet *temp = (struct TempSolutionsSet *)arg;
    int start = temp->k*NTHREADS, end = (temp->k + 1)*NTHREADS;
    for(int i = start; i < end; i++){
        if(graph[temp->tempSolutionsSet[i]][temp->vertex] != 0){
            flag = 0;
            pthread_exit(NULL);
        }
    }
}

void IndependentSets(int src, struct TempSolutionsSet temp){
    pthread_t threads[NTHREADS];
    int i;
    for(i = src; i < VERTICES; i++){
        temp.vertex = i;
        for(int k = 0; k < temp.size/NTHREADS; k++){
            temp.k = k;
            pthread_create(&threads[k], NULL, isSafeForIndependentSet, &temp);
        }
        for(int l = 0; l < temp.size/NTHREADS; l++)
            pthread_join(threads[l], NULL);
        if(temp.size < NTHREADS){
            for(int k = 0; k < temp.size; k++){
                if(graph[temp.tempSolutionsSet[k]][i] != 0){
                    flag = 0;
                }
            }
        }
        if(flag){
            temp.tempSolutionsSet[temp.size] = i;
            temp.size++;
            IndependentSets(i+1, temp);
            temp.size--;
        }
        flag = 1;
    }
    insert(temp);
}

void *mis(void *arg){
    int maxCount = 0;
    int *temp = (int *)arg;
    int start = (*temp)*NTHREADS, end = (*temp + 1)*NTHREADS;
    for(int i = start; i < end; i++){
        int localCount = independent_sets.independent_sets[i][0];
        if(localCount > maxCount)
            maxCount = localCount;
    }
    for(int i = start; i < end; i++){
        int localCount = independent_sets.independent_sets[i][0];
        if(localCount == maxCount){
            pthread_mutex_lock(&lock);
            int size = maximalindependentsets.size;
            maximalindependentsets.max_independent_sets[size][0] = 0;
            for(int k = 0; k < maxCount; k++){
                maximalindependentsets.max_independent_sets[size][k+1] = independent_sets.independent_sets[i][k+1];
                maximalindependentsets.max_independent_sets[size][0]++;
            }
            maximalindependentsets.size = size + 1;
            pthread_mutex_unlock(&lock);
        }
    }
}

void MaximalIndependentsets(){
    pthread_t thread[NTHREADS];
    for(int i = 0; i < independent_sets.size/NTHREADS; i++){
        pthread_create(&thread[i], NULL, mis, &i);
    }
    for(int l = 0; l < independent_sets.size/NTHREADS; l++)
        pthread_join(thread[l], NULL);
    if(independent_sets.size <= NTHREADS){
        int maxCount = 0;   
        for(int i = 0; i < independent_sets.size-1; i++){
        int localCount = independent_sets.independent_sets[i][0];
        if(localCount > maxCount)
            maxCount = localCount;
        }
        for(int i = 0; i < independent_sets.size-1; i++){
            int localCount = independent_sets.independent_sets[i][0];
            if(localCount == maxCount){
                int size = maximalindependentsets.size;
                maximalindependentsets.max_independent_sets[size][0] = 0;
                for(int k = 0; k < maxCount; k++){
                    maximalindependentsets.max_independent_sets[size][k+1] = independent_sets.independent_sets[i][k+1];
                    maximalindependentsets.max_independent_sets[size][0]++;
                }
                maximalindependentsets.size = size + 1;
            }
        }
    }
    int max_count = 0;
    for(int i = 0; i < maximalindependentsets.size-1; i++){
        int localCount = maximalindependentsets.max_independent_sets[i][0];
        if(localCount > max_count)
            max_count = localCount;
    }
    if(max_count == 0){
        printf("No maximal independent sets\n");
        return ;
    }
    printf("Maximal independent sets: \n");
    for(int i = 0; i < maximalindependentsets.size-1; i++){
        int localCount = maximalindependentsets.max_independent_sets[i][0];
        if(localCount == max_count){
            printf("{ ");
            for(int k = 1; k <= localCount; k++)
                printf("%d ", maximalindependentsets.max_independent_sets[i][k]);
            printf("}\n");
        }
    }
}   

int main(){
    srand(time_seed());
    int i, j;
    for(i=0; i<VERTICES; i++){ 
		for(j= i+1; j<VERTICES; j++){
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
    printf("\n");
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    struct TempSolutionsSet temp;
    temp.size = 0;
    IndependentSets(0, temp);
    MaximalIndependentsets();
}