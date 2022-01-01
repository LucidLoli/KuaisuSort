#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define CAPACITY 100000000

int* arr;
int* chunk;
int* sampling;
int** sendbuf;
int* newChunk;
int newChunkSize;
int* newArr;
int size, chunkSize, id, numProc, root;
struct timeval t0, t1;

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

int* binarySearch(int* begin, int* end, int target) {
  while (end - begin > 1) {
    int* mid = begin + (end - begin) / 2;
    if (*mid < target) begin = mid;
    else end = mid;
  }
  return begin;
}

void quickSortChunk(int* begin, int* end) {
  int stackSize = 0;
  int len = end - begin;
  if (len < 2) return;
  int** stack = (int**) malloc(sizeof(int*) * len * 4);
#define PUSH(a, b) do{stack[stackSize++]=(a);stack[stackSize++]=(b);}while(0)
#define POP() do{end=stack[--stackSize];begin=stack[--stackSize];}while(0)
  PUSH(begin, begin + len);
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
  free(stack);
#undef POP
#undef PUSH
}

int input(char* path, int* arr) {
  int cnt = 0;
  FILE* fp = path == NULL ? stdin : fopen(path, "r");
  while (fscanf(fp, "%x", arr + cnt) > 0) cnt++;
  if (path != NULL) fclose(fp);
  return cnt;
}

void samp() {
  int* p = (int*) malloc(sizeof(int) * numProc * 2);
  int i;
  for (i = 0; i < numProc; i++) p[i] = chunk[i * chunkSize / numProc];
  MPI_Gather(p, numProc, MPI_INT, sampling, numProc, MPI_INT, root, MPI_COMM_WORLD);
  free(p);
  if (id == root) {
    quickSortChunk(sampling, sampling + numProc * numProc);
    int i;
    for (i = 1; i < numProc; i++) sampling[i - 1] = sampling[i * numProc];
  }
}

void split(int* segs) {
  int i;
  int* left = chunk;
  int* right = chunk + chunkSize;
  for (i = 0; i < numProc - 1; i++) {
    right = binarySearch(left, chunk + chunkSize, sampling[i]) + 1;
    segs[i] = right - left;
    sendbuf[i] = left;
    left = right;
  }
  segs[i] = chunk + chunkSize - left;
  sendbuf[i] = left;
}

void exchange() {
  int i;
  int* seg = (int*) malloc(sizeof(int) * numProc);
  split(seg);
  int* cnt = (int*) malloc(sizeof(int) * numProc);
  int* pos = (int*) malloc(sizeof(int) * numProc);
  for (i = 0; i < numProc; i++)
    MPI_Gather(seg + i, 1, MPI_INT, cnt, 1, MPI_INT, i, MPI_COMM_WORLD);
  pos[0] = 0;
  for (i = 1; i < numProc; i++) pos[i] = pos[i - 1] + cnt[i - 1];
  newChunkSize = 0;
  for (i = 0; i < numProc; i++) newChunkSize += cnt[i];
  if (newChunkSize > 0) newChunk = (int*) malloc(sizeof(int) * newChunkSize);
  else newChunk = (int*) malloc(sizeof(int));
  MPI_Barrier(MPI_COMM_WORLD);
  for (i = 0; i < numProc; i++)
    MPI_Gatherv(sendbuf[i], seg[i], MPI_INT, newChunk, cnt, pos, MPI_INT, i, MPI_COMM_WORLD);
  free(seg);
  free(cnt);
  free(pos);
}

void output(char* path, int* arr, int len) {
  int i;
  FILE* fp = path == NULL ? stdout : fopen(path, "w");
  for (i = 0; i < len; i++) {
    fprintf(fp, "%x", arr[i]);
    if (i % 15 == 14) fprintf(fp, "\n");
    else fprintf(fp, " ");
  }
  if (path != NULL) fclose(fp);
}

void merge() {
  int* lens = NULL, * pos = NULL, i;
  if (id == root) {
    lens = (int*) malloc(sizeof(int) * numProc);
    pos = (int*) malloc(sizeof(int) * numProc);
    pos[0] = 0;
  }
  MPI_Gather(&newChunkSize, 1, MPI_INT, lens, 1, MPI_INT, root, MPI_COMM_WORLD);
  if (id == root) for (i = 1; i < numProc; i++) pos[i] = pos[i - 1] + lens[i - 1];
  MPI_Gatherv(newChunk, newChunkSize, MPI_INT, newArr, lens, pos, MPI_INT, root, MPI_COMM_WORLD);
  free(lens);
  free(pos);
}

int main(int argc, char** argv) {
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProc);
  if (numProc < 2) {
    if (id == 0) fprintf(stderr, "Too few processes. Nothing to do.\n");
    MPI_Finalize();
    return 0;
  }
  root = numProc - 1;
  if (id == root) {
    arr = (int*) malloc(sizeof(int) * CAPACITY);
    size = input(argv[1], arr);
    if (size < numProc * 2) {
      fprintf(stderr, "Too many processes. Nothing to do.\n");
    }
    fprintf(stderr, "Process %d: Data Loaded.\n", id);
    gettimeofday(&t0, NULL);
    newArr = (int*) malloc(sizeof(int) * size);
  }
  sampling = (int*) malloc(sizeof(int) * numProc * numProc);
  sendbuf = (int**) malloc(sizeof(int*) * numProc);
  memset(sendbuf, 0, sizeof(int*) * numProc);
  MPI_Bcast(&size, 1, MPI_INT, root, MPI_COMM_WORLD);
  chunkSize = size / numProc + (size % numProc ? 1 : 0);
  if (id == root) chunkSize -= chunkSize * numProc - size;
  chunk = (int*) malloc(sizeof(int) * chunkSize);
  MPI_Scatter(arr, chunkSize, MPI_INT, chunk, chunkSize, MPI_INT, root, MPI_COMM_WORLD);

  quickSortChunk(chunk, chunk + chunkSize);

  MPI_Barrier(MPI_COMM_WORLD);

  samp();

  MPI_Bcast(sampling, numProc - 1, MPI_INT, root, MPI_COMM_WORLD);
  exchange();
  quickSortChunk(newChunk, newChunk + newChunkSize);

  MPI_Barrier(MPI_COMM_WORLD);

  merge();
  if (id == root) {
    gettimeofday(&t1, NULL);
    fprintf(stderr, "Process %d: %lf s\nProcess %d: Printing Data.\n",
      id, t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0, id);
    output(argc > 2 ? argv[2] : NULL, newArr, size);
    free(arr);
    free(newArr);
  }
  free(sampling);
  free(chunk);
  free(sendbuf);
  MPI_Finalize();
  return 0;
}
