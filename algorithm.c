/**
 * Baseds on the algorithms written by Fabrice Bellard on January 8, 1998,
 * which were based off methods described by Simon Plouffe in "On the Computation
 * of the n'th decimal digit of various transcendental numbers' (November 1996)."
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "algorithm.h"

/**
 * Returns the inverse of x mod(y).
 * @param x Some integer X
 * @param y Some integer Y
 * @return The inverse of x mod(y)
 */
int inv_mod(int x, int y)
{
	int q, u, v, a, c, t;

	u = x;
	v = y;
	c = 1;
	a = 0;
	do {
		q = v / u;

		t = c;
		c = a - q * c;
		a = t;

		t = u;
		u = v - q * u;
		v = t;
	} while (u != 0);

	a = a % y;
	if (a < 0)
		a = y + a;

	return a;
}

/**
 * Returns a XOR b mod(m)
 * @param a Some integer A
 * @param b Some integer B
 * @param m Some integer M
 * @return a XOR (b mod(m))
 */
int pow_mod(int a, int b, int m)
{
	int r, aa;
	r = 1;
	aa = a;

	while (1) {
		if (b & 1)
			r = mul_mod(r, aa, m);
		b = b >> 1;
		if (b == 0)
			break;
		aa = mul_mod(aa, aa, m);
	}
	return r;
}

/**
 * Determines if a given integer is a prime number.
 * @param n Some integer N
 * @return True for prime numbers, otherwise false.
 */
bool is_prime(int n)
{
	int r, i;

	if ((n % 2) == 0)
		return false;

	r = (int) (sqrt(n));
	for (i = 3; i <= r; i += 2)
		if ((n % i) == 0)
			return false;

	return true;
}

/**
 * Find the next prime number after some integer n.
 * @param n The integer to find a prime after.
 * @return The next prime number after n.
 */
int next_prime(int n)
{
	do {
		n++;
	} while (! is_prime(n));
	return n;
}

/**
 * Finds the nth digit of Pi.
 * @param n The digit number to find.
 * @return The integer representation of the digit.
 */
int get_nth_digit(unsigned int n)
{
	int av, a, vmax, N, num, den, k, kq, kq2, t, v, s, i, result;
	double sum = 0;

	N = (int)((n + 20) * log(10) / log(2));

	for (a = 3; a <= (2 * N); a = next_prime(a)) {
		av = 1;
		s = 0;
		num = 1;
		den = 1;
		v = 0;
		kq = 1;
		kq2 = 1;

		vmax = (int) (log(2 * N) / log(a));
		for (i = 0; i < vmax; i++)
			av = av * a;

		for (k = 1; k <= N; k++) {
			t = k;
			if (kq >= a) {
				do {
					t = t / a;
					v--;
				} while ((t % a) == 0);
				kq = 0;
			}
			kq++;
			num = mul_mod(num, t, av);

			t = (2 * k - 1);
			if (kq2 >= a) {
				if (kq2 == a) {
					do {
						t = t / a;
						v++;
					} while ((t % a) == 0);
				}
				kq2 -= a;
			}
			den = mul_mod(den, t, av);
			kq2 += 2;

			if (v > 0) {
				t = inv_mod(den, av);
				t = mul_mod(t, num, av);
				t = mul_mod(t, k, av);
				for (i = v; i < vmax; i++)
					t = mul_mod(t, a, av);
				s += t;
				if (s >= av)
					s -= av;
			}

		}

		t = pow_mod(10, n - 1, av);
		s = mul_mod(s, t, av);
		sum = fmod(sum + (double) s / (double) av, 1.0);
	}

	result = (int)(sum * 1e9);

	//If result is less than 10x10^7, the first digit is always zero.
	if (result < 10 * 10000000)
		result = 0;
	else
		while(result >= 10)
			result = result / 10;

	return result;

}

void get_nth_series(unsigned int start, unsigned int end, short *store)
{
	unsigned short i = 0;
	for (; start<=end; ++start)
	{
		store[i] = get_nth_digit(start);
		++i;
	}
}
