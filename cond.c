#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define int int32_t
#define SIZE 100000000

int arr[SIZE];
int* stack[SIZE];
volatile int stackSize;
int len;
int nth;
volatile int waiting;
pthread_t* threads;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int* getPivot(int* begin, int* end) {
  int* c = begin + (end - begin) / 2;
  --end;
  if (*c <= *begin && *c <= *end)
    return *begin < *end ? begin : end;
  if (*begin <= *c && *begin <= *end)
    return *c < *end ? c : end;
  return *c < *begin ? c : begin;
}

int* partition(int* begin, int* end) {
    int _;
#define SWAP(a, b) (a!=b)&&(_=a,a=b,b=_)
    int* left = begin;
    int* right = end - 1;
    int* pivot = getPivot(begin, end);

    SWAP(*pivot, *right);
    pivot = right;
    while (left < right) {
        while (*left < *pivot && left < right) ++left;
        while (*right >= *pivot && left < right) --right;
        SWAP(*left, *right);
    }
    SWAP(*left, *pivot);
    return left;
#undef SWAP
}

void push(int* left, int* right) {
    pthread_mutex_lock(&mutex);
    stack[stackSize++] = left;
    stack[stackSize++] = right;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cond);
}

int pop(int** left, int** right) {
    pthread_mutex_lock(&mutex);
    if (stackSize == 0 && waiting + 1 == nth) {
        stackSize -= nth;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    ++waiting;
    while (stackSize == 0) pthread_cond_wait(&cond, &mutex);
    if (stackSize < 0 && waiting + 1 == nth) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    --waiting;
    *right = stack[--stackSize];
    *left = stack[--stackSize];
    pthread_mutex_unlock(&mutex);
    return 0;
}

void* subthreadRoutine(void* args) {
    int* begin, * end;
    while (1) {
        if (pop(&begin, &end) != 0) break;
        while (1) {
            int* p = partition(begin, end);
            int l = p - begin, r = end - p - 1;
            if (l > 1 && r > 1) {
                if (l > r) {
                    push(begin, p);
                    begin = p + 1;
                }
                else {
                    push(p + 1, end);
                    end = p;
                }
            }
            else {
                if (l <= 1 && r <= 1) break;
                if (l > 1) end = p;
                else begin = p + 1;
            }
        }
    }
    return NULL;
}

void quickSort() {
    for (int i = 1; i < nth; i++)
        pthread_create(threads + i - 1, NULL, subthreadRoutine, NULL);
    push(arr, arr + len);
    subthreadRoutine(NULL);
    for (int i = 1; i < nth; i++) pthread_join(threads[i - 1], NULL);
}

int main(int argc, char** argv) {
    int i;
    int* p = arr;
    FILE* fp = stdin;

    if (argc > 1) sscanf(argv[1], "%u", &nth);
    if (nth < 1) nth = 2;
    threads = (pthread_t*) malloc(sizeof(pthread_t) * (nth - 1));

    if (argc > 2) fp = fopen(argv[2], "r");

    while (fscanf(fp, "%x", p++) > 0) ++len;

    if (fp != stdin)fclose(fp);

    fprintf(stderr, "Data Loaded.\n");

    struct timeval start, end;
    gettimeofday(&start, NULL);
    quickSort();
    gettimeofday(&end, NULL);
    fprintf(stderr, "%lf s\n",
        end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0);

    fprintf(stderr, "Printing Data.\n");

    for (i = 0; i < len; i++) {
        printf("%x", arr[i]);
        if (i % 15 == 14) printf("\n");
        else printf(" ");
    }

    free(threads);

    return 0;
}

#undef int
