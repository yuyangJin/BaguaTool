#include<stdio.h>

double A[1000][1000] = {0};
double B[1000] = {0};
double C[1000] = {0};

int print(int counter){
  printf("counter = %d\n",counter);
}

int bar(int j){
  double x, y = 0;
  x = j + 2.4;
  y = j / 3.7;
  A[j][j] = x - y;
  B[j] = y * x;

  return A[j][j]+B[j];
}

int foo(int i){
  int sum = 0;
  //double x = 0;
  for (int j = i; j < i; i++){
    for (int k = j; k < j; k++){
      sum += A[i][k] * B[k];
    }
    sum += bar(j);
    C[j] += sum * 1.4;
  }
  //printf("foo sum = %d",sum);
  return sum;
}

int main(){
  int sum = 0;
  for(int i = 0; i < 100000; i++){
    sum += foo(i);
  }
  //print(sum);
}