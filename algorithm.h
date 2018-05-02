#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <math.h>
#include <stdbool.h>

#define mul_mod(a, b, m) fmod((double)a * (double) b, m)

int inv_mod(int x, int y);
int pow_mod(int a, int b, int m);
bool is_prime(int n);
int next_prime(int n);
int get_nth_digit(unsigned int n);
void get_nth_series(unsigned int start, unsigned int end, short *store);

#endif
