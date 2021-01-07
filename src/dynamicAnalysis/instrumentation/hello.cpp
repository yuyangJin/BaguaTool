#include<stdio.h>

int counter_foo = 0, counter_bar = 0;
int bar(int j){
  counter_bar ++;
  return j*j/2;
}

int foo(int i){
  int sum = 0;
  for (int j = i; j < 10; j++){
    for (int k = j; k < 100; k++){
      sum += bar(j);
    }
  }
  counter_foo ++;
  //printf("foo sum = %d",sum);
  return sum;
}

int main(){
  int sum = 0;
  //for(int i = 0; i < 1000; i++){
  sum += foo(0);
  //}
  printf("counter_bar = %d\n", counter_bar);
  printf("counter_foo = %d\n", counter_foo);
}