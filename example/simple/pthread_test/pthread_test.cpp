#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
//#ifndef N
//#define N 5000000
//#endif
int N = 5000000;
typedef struct ct_sum {
  int sum;
  pthread_mutex_t lock;
} ct_sum;
void *add1(void *cnt) {
  double a = 0;
  for (int i = 0; i < 50; i++) {
    a += i;
  }
  struct timeval start;
  struct timeval end;
  unsigned long diff;
  gettimeofday(&start, NULL);
  pthread_mutex_lock(&(((ct_sum *)cnt)->lock));
  gettimeofday(&end, NULL);
  diff = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
  printf("add1 lock is %ld\n", diff);
  int i;
  gettimeofday(&start, NULL);
  for (i = 0; i < N; i++) {
    (*(ct_sum *)cnt).sum += i;
  }
  gettimeofday(&end, NULL);
  diff = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
  printf("add1 for is %ld\n", diff);
  pthread_mutex_unlock(&(((ct_sum *)cnt)->lock));
  pthread_exit(NULL);
  return 0;
}
void *add2(void *cnt) {
  struct timeval start;
  struct timeval end;
  unsigned long diff;
  gettimeofday(&start, NULL);
  pthread_mutex_lock(&(((ct_sum *)cnt)->lock));
  gettimeofday(&end, NULL);
  diff = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
  printf("add2 lock is %ld\n", diff);
  int i;
  gettimeofday(&start, NULL);
  for (i = N; i < N * 2 + 1; i++) {
    (*(ct_sum *)cnt).sum += i;
  }
  gettimeofday(&end, NULL);
  diff = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
  printf("add2 for is %ld\n", diff);
  pthread_mutex_unlock(&(((ct_sum *)cnt)->lock));
  pthread_exit(NULL);
  return 0;
}
int main(int argc, char **argv) {
  N = atoi(argv[1]);
  int i;

  pthread_t ptid1, ptid2;
  int sum = 0;
  ct_sum cnt;
  pthread_mutex_init(&(cnt.lock), NULL);
  cnt.sum = 0;

  for (i = 0; i < N / 2; i++) {
    cnt.sum += i;
  }

  pthread_create(&ptid1, NULL, add1, &cnt);
  pthread_create(&ptid2, NULL, add2, &cnt);

  pthread_mutex_lock(&(cnt.lock));
  printf("sum %d\n", cnt.sum);
  pthread_mutex_unlock(&(cnt.lock));

  pthread_join(ptid1, NULL);
  pthread_join(ptid2, NULL);

  printf("sum %d\n", cnt.sum);

  pthread_mutex_destroy(&(cnt.lock));
  return 0;
}
