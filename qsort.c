// C Source
// Sequential Quick Sort

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define int int32_t
#define SIZE 100000000

int arr[SIZE];
int* stack[SIZE];
int len;

int comp(const void* _a, const void* _b) {
  int a = *(int*)_a, b = *(int*)_b;
  if (a == b) return 0;
  return a > b ? 1 : -1;
}

int main(int argc, char const* argv[]) {
  int i;
  int* p = arr;

  while (scanf("%x", p++) > 0) ++len;

  fprintf(stderr, "Data Loaded.\n");

  struct timeval start, end;
  gettimeofday(&start, NULL);

  qsort(arr, len, sizeof(int), comp);
  gettimeofday(&end, NULL);

  fprintf(stderr, "%lf s\n", end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0);

  fprintf(stderr, "Printing Data.\n");

  for (i = 0; i < len; i++) {
    printf("%x", arr[i]);
    if (i % 15 == 14) printf("\n");
    else printf(" ");
  }

  return 0;
}

#undef int
