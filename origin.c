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

/**
 * @brief Compares @p *begin, @p *end, and the middle
 * one between the both.  Return the median of
 * the 3 numbers as the pivot for quick sort.
 *
 * @param begin One int32 pointer.
 * @param end Yet another int32 pointer.
 * @return The median of 3 numbers.
 */
int* getPivot(int* begin, int* end) {
  // to avoid the case where (begin + end) here overflows
  int* c = begin + (end - begin) / 2;
  --end;
  if (*c <= *begin && *c <= *end)
    return *begin < *end ? begin : end;
  if (*begin <= *c && *begin <= *end)
    return *c < *end ? c : end;
  return *c < *begin ? c : begin;
}

/**
 * @brief Moves the elements in @p [begin,end) so that
 * elements on the left of the pivot are all not larger
 * than the pivot and those on the right are all not
 * smaller than the pivot.
 *
 * @param begin One int32 pointer.
 * @param end Yet another int32 pointer.
 * @return the pointer to the pivot.
 */
int* partition(int* begin, int* end) {
#define SWAP(a, b) (a!=b)&&(_=a,a=b,b=_)
  int _;
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

/**
 * @brief Sorts the elements in @p arr in ascending order.
 * @return Nothing.
 */
void quickSort() {
  int stackSize = 0;
  int* begin;
  int* end;

  if (len < 2) return;

#define PUSH(a, b) do{stack[stackSize++]=(a);stack[stackSize++]=(b);}while(0)
#define POP() do{end=stack[--stackSize];begin=stack[--stackSize];}while(0)

  PUSH(arr, arr + len);
  while (stackSize) {
    POP();
    while (1) {
      int* pivot = partition(begin, end);
      int left = pivot - begin, right = end - pivot - 1;
      if (right > 1 && left > 1) {
        if (left < right) {
          PUSH(pivot + 1, end);
          end = pivot;
        }
        else {
          PUSH(begin, pivot);
          begin = pivot + 1;
        }
      }
      else {
        if (left <= 1 && right <= 1) break;
        if (left > 1) end = pivot;
        else begin = pivot + 1;
      }
    }
  }

#undef POP
#undef PUSH
}

int main(int argc, char const* argv[]) {
  int i;
  int* p = arr;

  while (scanf("%x", p++) > 0) ++len;

  fprintf(stderr, "Data Loaded.\n");

  struct timeval start, end;
  gettimeofday(&start, NULL);

  quickSort();
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
