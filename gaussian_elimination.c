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
#define MAXN 2000

int N = 10;  
int NTHREADS = 5; 
pthread_mutex_t lck;
int max_col = INT_MIN;
int max_loc = 0;

volatile float A[MAXN][MAXN], B[MAXN], X[MAXN];

void gauss();  

unsigned int time_seed() {
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}


struct threadData{
   int norm;
   int i;
};


void initialize_inputs() {
    int row, col;

    printf("\nInitializing...\n");
    for (col = 0; col < N; col++) {
        for (row = 0; row < N; row++) {
        A[row][col] = (float)rand() / 32768.0;
        }
        B[col] = (float)rand() / 32768.0;
        X[col] = 0.0;
    }
}

void print_inputs() {
    int row, col;
    if (N < 10) {
        printf("\nA =\n\t");
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
            printf("%5.2f%s", A[row][col], (col < N-1) ? ", " : ";\n\t");
            }
        }
            printf("\nB = [");
        for (col = 0; col < N; col++) {
            printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
        }
    }
}

void print_X() {
    int row;
    if (N < 100) {
        printf("\nX = [");
        for (row = 0; row < N; row++) {
            printf("%5.2f%s", X[row], (row < N-1) ? "; " : "]\n");
        }
    }
}

void swap_col(int col){
    int temp = B[col];
    B[col] = B[0];
    B[0] = temp;
    for(int i = 0; i < N; i++){
        temp = A[i][col];
        A[i][col] = A[0][col];
        A[0][col] = temp;
    }
}

void *inner_loop(void * param){
    struct threadData* tparam = (struct threadData*) param;
    int norm = tparam->norm;
    int i = tparam->i;
    float multiplier;
    int row, col;
    for (row = norm + i + 1; row < N; row = row + NTHREADS) {
        multiplier = A[row][norm] / A[norm][norm];
        for (col = norm; col < N; col++) {
            A[row][col] -= A[norm][col] * multiplier;
        }
        B[row] -= B[norm] * multiplier;
    }
    pthread_exit(0);
}

void *calculate_max(void *arg){
    struct threadData *tparam = (struct threadData *)arg;
    int norm = tparam->norm;
    int start = (tparam->i)*NTHREADS, end = (tparam->i + 1)*NTHREADS;
    int maxi =  INT_MIN;
    int col, loc = 0;
    for(col = start; col < end; col++){
        if(A[norm][col] > maxi){
            maxi = A[norm][col];
            loc = col;
        }
    }
    pthread_mutex_lock(&lck);
    if(max_col < maxi){
        max_col = maxi;
        max_loc = loc;
    }
    pthread_mutex_unlock(&lck);
}

void gauss() {
    int norm, row, col;  
    float multiplier;

    pthread_t thread[N];
    for (norm = 0; norm < N - 1; norm++) {
        struct threadData* param = malloc(NTHREADS*sizeof(struct threadData));
        if ( param == NULL ) {
            fprintf(stderr, "Couldn't allocate memory for thread.\n");
            exit(EXIT_FAILURE);
        }

        int i, j;
        /* partial pivoting*/
        for(i = 0; i < N/NTHREADS; i++){
            param[i].norm = norm;
            param[i].i = i;
            pthread_create(&thread[i], NULL, calculate_max, &param[i]);
        }
        for (j = 0; j < N/NTHREADS; j++) {
            pthread_join(thread[j], NULL);
        }
        if(max_loc != 0){
            swap_col(max_loc);
        }
        for(i = 0; i < NTHREADS; i++){
            param[i].norm = norm;
            param[i].i = i;
            pthread_create(&thread[i], NULL, inner_loop, (void*) &param[i]);
        }

        for (j = 0; j < NTHREADS; j++) {
            pthread_join(thread[j], NULL);
        }

        free(param);
    }

    /* Back substitution */
    for (row = N - 1; row >= 0; row--) {
        X[row] = B[row];
        for (col = N-1; col > row; col--) {
        X[row] -= A[row][col] * X[col];
        }
        X[row] /= A[row][row];
    }
}

int main(int argc, char **argv) {  
    srand(time_seed());
    pthread_mutex_init(&lck, NULL);
    initialize_inputs();
    print_inputs();
    gauss();
    print_X();
}

