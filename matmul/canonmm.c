/*

CANON'S Matrix Multiplication Algorithm.


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
#include <string.h>

#define MAX 10
  
#define NTHREAD 5 
  
int matA[MAX][MAX]; 
int matB[MAX][MAX]; 
int matC[MAX][MAX]; 

unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}


struct threadData{
    int flag;
    int j;
};

void print_A(){
    printf("\nMatrix A\n");
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            printf("%d ", matA[i][j]); 
        printf("\n"); 
    }
}

void print_B(){
    printf("\nMatrix B\n");  
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            printf("%d ", matB[i][j]); 
        printf("\n"); 
    }
}

void print_C(){
    printf("\nMatrix C\n");  
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            printf("%d ", matC[i][j]); 
        printf("\n"); 
    }
}

void lshift(int r, int s){
    for(int i = 0; i < s; i++){
        int a = matA[r][0];
        for(int j = 1; j < MAX; j++){
            matA[r][j-1] = matA[r][j];
        }
        matA[r][MAX-1] = a;
    }
}

void ushift(int c, int s){
    for(int i = 0; i < s; i++){
        int a = matB[0][c];
        for(int j = 1; j < MAX; j++){
            matB[j-1][c] = matB[j][c];
        }
        matB[MAX-1][c] = a;
    }
}

void *multi(void *arg){
    struct threadData *temp = (struct threadData *)arg;
    int start = temp->j*NTHREAD, end = (temp->j + 1)*NTHREAD; 
    int flag = temp->flag;
    for(int i = start; i < end; i++){
        int c;
        if(flag == 1)c = i;
        else c = 1;
        lshift(i, c);
        ushift(i, c);
    }       
}

int main(){
    srand(time_seed());
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++) { 
            matA[i][j] = rand() % 10; 
            matB[i][j] = rand() % 10; 
        } 
    }  
    print_A();
    print_B();
    memset(matC, 0, sizeof(matC));
  
    for(int i = 0; i < MAX; i++){
        pthread_t thread[NTHREAD];
        for(int j = 0; j < MAX/NTHREAD; j++){
            struct threadData t;
            t.flag = 0;
            if(i == 0)t.flag = 1;
            t.j = j; 
            pthread_create(&thread[j], NULL, multi, &t);
        }
        for(int l = 0; l < MAX/NTHREAD; l++)
            pthread_join(thread[l], NULL);

        for(int k = 0; k < MAX; k++){
            for(int l = 0; l < MAX; l++){
                matC[k][l] += matA[k][l]*matB[k][l]; 
            }
        }
    }    

    printf("\nMultiplication of A and B\n");
    for (int i = 0; i < MAX; i++) { 
        for (int j = 0; j < MAX; j++)  
            printf("%d ", matC[i][j]);         
        printf("\n"); 
    }  
}