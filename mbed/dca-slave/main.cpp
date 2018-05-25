/**
* The Mbed online compiler only gives limited debug information on the
* compiler and linker which makes proper code structure difficult.
* All files were combined for compilation purposes.
*/
#include "mbed.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <stdbool.h>

#define mul_mod(a, b, m) fmod((double)a * (double) b, m)

#define EFP_CMD_REGISTER_BYTE 0x0
#define EFP_CMD_REGISTER_SLAVE_ACK_BYTE 0x1
#define EFP_CMD_REGISTER_DATA_BYTE 0x2
#define EFP_SLAVE_ADDR 0x10
#define EFP_SLAVE_REGISTERS 0x2
#define EFP_JOB_FACTOR 0x5
#define EFP_ACK_OK 0x1
#define EFP_ACK_ERR 0x2

typedef enum
{
	EFP_CMD_PING = 0x0,
	EFP_CMD_ORDER = 0x1,
	EFP_CMD_STATUS = 0x2,
	EFP_CMD_RESULT = 0x3,
	EFP_CMD_RESET = 0x4
} EFP_CMD;

typedef enum
{
	EFP_MODE_IDLE,
	EFP_MODE_WORK,
	EFP_MODE_DONE
} EFP_MODE;

typedef struct
{
	EFP_MODE mode;
	uint8_t current_job_start_idx;
	uint8_t current_job_progress;
	uint8_t current_job_results[EFP_JOB_FACTOR];
	uint16_t reg_val;
	char registers[6];
} efp_slave;

//We need a compute thread just like the photon.
Thread compute_thread;

/**
* Sets the current job index of the efp slave.
* @param slave     A pointer to the efp_slave
* @param start_idx The start index for the job group
*/
void efp_set_job(efp_slave *slave, const uint8_t start_idx)
{
	slave->current_job_start_idx = start_idx;
	slave->current_job_progress = 0x0;
	for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
	slave->current_job_results[i] = 0x0;
	slave->mode = EFP_MODE_WORK;
}

/**
* Set the slave to computation done status.
* @param slave A pointer to the efp_slave
*/
void efp_set_done(efp_slave *slave)
{
	slave->mode = EFP_MODE_DONE;
}

/**
* Set the slave to idle status.
* @param slave A pointer to the efp_slave
*/
void efp_set_idle(efp_slave *slave)
{
	slave->mode = EFP_MODE_IDLE;
}

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

	r = (int) (sqrt((double)n));
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

	N = (int)((n + 20) * log(10.0) / log(2.0));

	for (a = 3; a <= (2 * N); a = next_prime(a)) {
		av = 1;
		s = 0;
		num = 1;
		den = 1;
		v = 0;
		kq = 1;
		kq2 = 1;

		vmax = (int) (log(2.0 * N) / log((double)a));
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

	return (uint8_t)result;

}

I2CSlave slave(p9, p10);
static efp_slave slave_efp;


void compute()
{
	while (1)
	{
		if (slave_efp.mode != EFP_MODE_WORK)
		{
			//printf("Waiting for work...\r\n");
			Thread::wait(50);
			//delay(50);
			//os_thread_yield();
			continue;
		}

		int start = slave_efp.current_job_start_idx * EFP_JOB_FACTOR +1;
		int end = start + (EFP_JOB_FACTOR -1);
		int n = start;
		printf("Computing %i to %i\r\n", start, end);
		//Thread::wait(500);

		for (slave_efp.current_job_progress=0; n<=end; ++slave_efp.current_job_progress)
		{
			slave_efp.current_job_results[slave_efp.current_job_progress] = get_nth_digit(n++);
			//os_thread_yield();
			//printf("Result: %u\r\n", slave_efp.current_job_results[slave_efp.current_job_progress]);
		}


		slave_efp.mode = EFP_MODE_DONE;
		printf("Digit computation done.\r\n");
		for (int x=0; x<EFP_JOB_FACTOR; ++x)
		printf("%i", slave_efp.current_job_results[x]);
		printf("\r\n");
		//os_thread_yield();

	}
}

/**
* The main entrypoint for Mbed.
* @return int     The status code.
*/
int main() {
	//Setup an input buffer used for parsing.
	char input_buffer[6];

	//And our own 6 byte register.
	//Mbed doesn't have the luxury of in-built I2C registers like Photon,
	//so instead we replicate it.
	char r1[6];

	//Init register.
	r1[0] = 0x00;
	r1[1] = 0x00;
	r1[2] = 0x00;
	r1[3] = 0x00;
	r1[4] = 0x00;
	r1[5] = 0x00;

	compute_thread.start(compute);

	//Set the slave address to 0xA0.
	//Note that Mbed uses 7-bit addressing, whereas Photon and Linux use 8-bit.
	//To access this on Linux, 0xA0 needs to be shifted 1 bit to the right.
	slave.address(0xA0);

	//Main loop.
	//We never break out of here.
	while (1) {

		int i = slave.receive();
		switch (i) {
			//The master has requested data.
			case I2CSlave::ReadAddressed:
				//Write the contents of register 1 over I2C.
				slave.write(r1, 6);

				//Reset the register back to 0.
				for (int i=0; i<6; ++i)
					r1[i] = 0x0;

			break;
			//The master has written data.
			case I2CSlave::WriteAddressed:
				//Read the data into the input buffer directly.
				slave.read(input_buffer, 6);

				//Move them to the efp register.
				for (int i=0; i<6; ++i) {
					r1[i] = input_buffer[i];
				}

				printf("Command received from master!\r\n");
				switch (r1[EFP_CMD_REGISTER_BYTE +2])
				{
					case EFP_CMD_PING:
						printf("Ping!\r\n");
						r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
					break;
					case EFP_CMD_ORDER:
						printf("Work order\r\n");

						if (slave_efp.mode != EFP_MODE_IDLE)
						{
							printf("Cannot accept work, not in idle mode.\r\n");
							r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_ERR;
						}
						else
						{
							uint8_t work_value = r1[EFP_CMD_REGISTER_DATA_BYTE + 2];
							printf("Requested work value is: %u\r\n", work_value);

							efp_set_job(&slave_efp, work_value);
							r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
						}

					break;
					case EFP_CMD_STATUS:
						printf("Check status\r\n");
						r1[EFP_CMD_REGISTER_DATA_BYTE] = slave_efp.current_job_progress;
						r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
					break;
					case EFP_CMD_RESULT:
						printf("Request result\r\n");
						if (slave_efp.mode != EFP_MODE_DONE)
						{
							printf("Cannot give results while still computing\r\n");
							r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_ERR;
						}
						else
						{
							uint8_t idxRequested = r1[EFP_CMD_REGISTER_DATA_BYTE + 2];
							if (idxRequested > EFP_JOB_FACTOR || idxRequested <= 0)
							{
								printf("The requested result index is greater than EFP job factor. No buffer overflows here!\r\n");
								r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_ERR;
							}
							else
							{
								r1[EFP_CMD_REGISTER_DATA_BYTE] = slave_efp.current_job_results[idxRequested -1];
								r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
							}
						}

					break;
					case EFP_CMD_RESET:
						printf("Reset\r\n");
						if (slave_efp.mode != EFP_MODE_DONE)
						{
							printf("Can only reset when done\r\n");
							r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_ERR;
						}
						else
						{
							r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
						}
						slave_efp.mode = EFP_MODE_IDLE;
					break;
				}

				slave.stop();
				break;
			}

			//Clear our input buffer before we loop back again.
			for(int i = 0; i < 10; i++)
				input_buffer[i] = 0;
	}
}
