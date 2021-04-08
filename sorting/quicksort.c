#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#define NN	0x500000
//#define N	0x1000
#define MINIMUMSIZE	1024
#define MAXSUBLISTS	(NN / MINIMUMSIZE)

struct sublist {
	int start;
	int end;
};

int N;
int P;
int args[64];

int A[NN];

struct sublist sublist_info[MAXSUBLISTS];

volatile int head, tail;

volatile int sort_complete, sleepcount;
pthread_mutex_t mut;
pthread_cond_t cond_empty, cond_full;
pthread_barrier_t barr;

void insert_sublist(int start, int end)
{
	pthread_mutex_lock(&mut);
	while (((tail + 1) % MAXSUBLISTS) == head)
	{
		pthread_cond_wait(&cond_empty, &mut);
	}

	sublist_info[tail].start = start;
	sublist_info[tail].end = end;

	tail = (tail + 1) % MAXSUBLISTS;
	pthread_cond_signal(&cond_full);
	pthread_mutex_unlock(&mut);
}

struct sublist remove_sublist(void)
{
	struct sublist retval;

	pthread_mutex_lock(&mut);
	while ((head == tail) && (!sort_complete))
	{
		sleepcount++;
		if (sleepcount < P)
		{
			pthread_cond_wait(&cond_full, &mut);
			sleepcount--;
		}
		else
		{
			sort_complete = 1;
			pthread_cond_broadcast(&cond_full);
		}
	}

	if (sort_complete == 0)
	{
		retval = sublist_info[head];
		head = (head + 1) % MAXSUBLISTS;		

		pthread_cond_signal(&cond_empty);
	}
	else
	{
		retval.start = 0;
		retval.end = 0;
	}

	pthread_mutex_unlock(&mut);

	return retval;
}

void insertionsort(int start, int end)
{
	int i, j, val;

	for (i=start+1; i<=end; i++)
	{
		val = A[i];
		j = i-1;
		while (j >= start && A[j] > val)
		{
			A[j+1] = A[j];
			j--;
		}
		A[j+1] = val;
	}
}

int partition(int start, int end)
{
	int i, j, pivotpos;
	int pivot = A[end];
	int done = 0, temp;

	i = start;
	j = end - 1;
	while (!done)
	{
		while (i < j && A[i] < pivot) i++;
		while (i < j && A[j] > pivot) j--;
		if (i < j)
		{
			temp = A[i];
			A[i] = A[j];
			A[j] = temp;
		}
		else
		{
			if (A[i] > pivot)
			{
				A[end] = A[i];
				A[i] = pivot;
				pivotpos = i;
			}
			else pivotpos = end;
			done = 1;
		}
	}

	return pivotpos;	
}

void quicksort(int start, int end)
{
	int i, n, pivot;
	int lo, hi;

quicksort_1:
	if (end <= start) return; 
	if ((tail % 50) == 0)
		printf("Quick sort called : %d, %d, (%d, %d)\n",start,end,head,tail);
	if ((end - start + 1) < MINIMUMSIZE)
	{
		insertionsort(start, end);
	}
	else
	{
		pivot = partition(start, end);
		if (pivot-1 > start && pivot+1 < end)
		{
			if ((pivot-start-1) < (end - pivot - 1))
			{
				insert_sublist(start, pivot-1); 
				start = pivot+1;
				goto quicksort_1;
			}
			else
			{
				insert_sublist(pivot+1, end); 
				end = pivot-1;
				goto quicksort_1;
			}
		}
		else
		{
			if (pivot-1 > start)
			{
		
				end = pivot-1;
				goto quicksort_1;
			}
			else if (pivot+1 < end)
			{
				start = pivot+1;
				goto quicksort_1;
			}
			else printf("Both sublists wont exist! Should not happen!\n");
		}
	}
}

void *thread_func(void *arg)
{
	struct sublist s;
	int done = 0;
	char mesg[100];
	int fd;
	int procno;

	procno = *(int *)arg;
	pthread_barrier_wait(&barr);
	printf("Thread id : %d - sublistlise : %d\n",procno, MAXSUBLISTS);
	if (procno == 0) {

		s.start = 0; 
		s.end = N-1;
	}
	else s = remove_sublist();
	do {
		if (s.start == 0 && s.end == 0) done = 1;
		else 
		{
			quicksort(s.start, s.end);
			s = remove_sublist();
		}
	} while (!done);
	printf("Terminating the thread function\n");
	return NULL;
}

int main(int argc, char **argv)
{
	int i,j, mcmodel;
	int t1, t2, t3, t4;
	int t;
	pthread_t thr[64];
	if (argc != 3){
		printf("Usage : quiksort <NPROC> <NELEMENTS>\n");
		exit(0);
	}
	P = atoi(argv[1]);
	N = atoi(argv[2]);
	pthread_barrier_init(&barr, NULL, P);
	sleepcount = 0;
	sort_complete = 0;
	for (i=0; i<64; i++) args[i] = i;
	pthread_mutex_init(&mut, NULL);
	pthread_cond_init(&cond_empty, NULL);
	pthread_cond_init(&cond_full, NULL);
	// Initialize and Shuffle
	for (i=0; i<N; i++)
		A[i] = i;
	j = 1137;
	for (i=0; i<N; i++){
		t = A[j];
		A[j] = A[i];
		A[i] = t;
		j = (j + 337) % N;
	}
	for (i = 0; i<N; i++)
		printf("%d ", A[i]);
	printf("\n");
	for (i=1; i<P; i++)
		pthread_create(&thr[i], NULL, thread_func, &args[i]);
	thread_func(&args[0]);
	for (i=1; i<P; i++)
		pthread_join(thr[i], NULL);
	for (i=0; i<N; i++)
		printf("%8d",A[i]);
	return 0;
}