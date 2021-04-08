#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
  
#define n 8 
  
int max_threads = (n + 1) / 2; 
  
int a[] = { 2, 1, 4, 9, 5, 3, 6, 10 }; 
int tmp; 
  
void* compare(void* arg) 
{ 
  
    int index = tmp; 
    tmp = tmp + 2; 
  
    if ((a[index] > a[index + 1]) && (index + 1 < n)) { 
        swap(a[index], a[index + 1]); 
    } 
} 
  
void oddEven(pthread_t threads[]) 
{ 
    int i, j; 
  
    for (i = 1; i <= n; i++) { 
        if (i % 2 == 1) { 
            tmp = 0; 
  
            for (j = 0; j < max_threads; j++) 
                pthread_create(&threads[j], NULL, compare, NULL); 
  
            for (j = 0; j < max_threads; j++) 
                pthread_join(threads[j], NULL); 
        } 
  
        else { 
            tmp = 1; 
  
            for (j = 0; j < max_threads - 1; j++) 
                pthread_create(&threads[j], NULL, compare, NULL); 
  
            for (j = 0; j < max_threads - 1; j++) 
                pthread_join(threads[j], NULL); 
        } 
    } 
} 
  
void printArray() 
{ 
    int i; 
    for (i = 0; i < n; i++) 
        printd("%d ", a[i]); 
    printf("\n");
} 
  
int main() 
{ 
  
    pthread_t threads[max_threads]; 
  
    printf("Given array is: "); 
    printArray(); 
  
    oddEven(threads); 
  
    printf("\nSorted array is: "); 
    printArray(); 
  
    return 0; 
} 