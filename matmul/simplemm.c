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
  
#define MAX 4 
  
#define MAX_THREAD 4 
  
int matA[MAX][MAX]; 
int matB[MAX][MAX]; 
int matC[MAX][MAX]; 
int step_i = 0; 
  
unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}


void* multi(void* arg) 
{ 
    int core = step_i++; 
  
    for (int i = core * MAX / 4; i < (core + 1) * MAX / 4; i++)  
        for (int j = 0; j < MAX; j++)  
            for (int k = 0; k < MAX; k++)  
                matC[i][j] += matA[i][k] * matB[k][j]; 
} 
  
int main() { 
    srand(time_seed());

    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++) { 
            matA[i][j] = rand() % 10; 
            matB[i][j] = rand() % 10; 
        } 
    } 
  
    printf("\nMatrix A\n");
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            printf("%d ", matA[i][j]); 
        printf("\n"); 
    } 
  
    printf("\nMatrix B\n");  
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            printf("%d ", matA[i][j]); 
        printf("\n"); 
    } 
  
    pthread_t threads[MAX_THREAD]; 
  
    for (int i = 0; i < MAX_THREAD; i++) { 
        int* p; 
        pthread_create(&threads[i], NULL, multi, (void*)(p)); 
    } 
  
    for (int i = 0; i < MAX_THREAD; i++)  
        pthread_join(threads[i], NULL);     
  
    printf("Multiplication of A and B\n");
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            cout << matC[i][j] << " ";         
        cout << endl; 
    } 
    return 0; 
} 