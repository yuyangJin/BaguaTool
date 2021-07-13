#include <stdio.h>
#include <stdlib.h>

// calculate the fact of n
unsigned long long fact(int n, int p) {
    if (n == 1) {
        return p;
    }
    return fact(n - 1, n * p);
}

int N = 30;

int main(int argc, char **argv) {
    N = atoi(argv[1]);
    printf("The result of %d is: %llu\n", N, fact(N, 1));
    return 0;
}