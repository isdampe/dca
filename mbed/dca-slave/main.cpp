#include <mbed.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mbed.h"
#include <cmath>
#include <stdbool.h>
#include "rtos.h"

#define mul_mod(a, b, m) fmod((double)a * (double) b, m)

//EFP - Efficient ........ protocol.
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

static Mutex register_lock;

void efp_slave_init(efp_slave *slave)
{

    register_lock.lock();
    slave->mode = EFP_MODE_IDLE;
    slave->current_job_start_idx = 0x0;
    slave->current_job_progress = 0x0;

    for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
        slave->current_job_results[i] = 0x0;

    for (uint8_t i=0; i<4; ++i)
        slave->registers[i] = 0x0;

    register_lock.unlock();
}


void efp_dump_registers(const efp_slave *slave)
{
    register_lock.lock();
    for (uint8_t i=0x0; i<0x4; ++i)
        printf("Reg %i: 0x%02x\r\n", i, slave->registers[i]);
    register_lock.unlock();
    printf("\r\n");
}

void efp_slave_parse_registers(const uint32_t reg_val, efp_slave *slave, uint8_t reg_addr)
{
    register_lock.lock();
    for (uint8_t i=0x0; i<0x4; ++i)
        slave->registers[i] = (reg_val >> (i * 8) & 0xff);
    register_lock.unlock();
    
    efp_dump_registers(slave);
}


uint32_t efp_pack_registers(const efp_slave *slave)
{
    uint32_t result = 0x0;
    register_lock.lock();
    for (int8_t i=0x3; i>=0; --i)
        result = (result << 8) | slave->registers[i];
    register_lock.unlock();
    return result;
}

void efp_set_ack(efp_slave *slave, const uint8_t value)
{
    register_lock.lock();
    slave->registers[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = value;
    register_lock.unlock();
}

uint8_t efp_get_register_byte(const efp_slave *slave, const uint8_t index)
{
    uint8_t result;
    register_lock.lock();
    result = slave->registers[index];
    register_lock.unlock();

    return result;
}

void efp_set_register_byte(efp_slave *slave, const uint8_t index, const uint8_t val)
{
    register_lock.lock();
    slave->registers[index] = val;
    register_lock.unlock();
}

void efp_set_job(efp_slave *slave, const uint8_t start_idx)
{
    register_lock.lock();
    slave->current_job_start_idx = start_idx;
    slave->current_job_progress = 0x0;
    for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
        slave->current_job_results[i] = 0x0;
    slave->mode = EFP_MODE_WORK;
    register_lock.unlock();
}

void efp_set_done(efp_slave *slave)
{
    register_lock.lock();
    slave->mode = EFP_MODE_DONE;
    register_lock.unlock();
}

void efp_set_idle(efp_slave *slave)
{
    register_lock.lock();
    slave->mode = EFP_MODE_IDLE;
    register_lock.unlock();
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


int main() {
    char input_buffer[6];
    char r1[6];
    
    //Init register.
    r1[0] = 0x00;
    r1[1] = 0x00;
    r1[2] = 0x00;
    r1[3] = 0x00;
    r1[4] = 0x00;
    r1[5] = 0x00;
    
   // printf("The 5th digit is %i\r\n", get_nth_digit(5));
   
   slave.address(0xA0);
   
   while (1) {
       int i = slave.receive();
       switch (i) {
           //Write the register to master.
           case I2CSlave::ReadAddressed:
               slave.write(r1, 6); // Includes null char
               for (int i=0; i<6; ++i)
                r1[i] = 0;
               break;
            //Not used for this implementation.
           case I2CSlave::WriteGeneral:
               //slave.read(input_buffer, 6);
               break;
            //On command from master!
           case I2CSlave::WriteAddressed:
               slave.read(input_buffer, 6);
               
               //Move them to the efp buffer
               for (int i=0; i<6; ++i) {
                    //slave_efp.registers[i] = input_buffer[i];
                    r1[i] = input_buffer[i];
                    printf("Reg %i: Byte: 0x%02x\r\n\r\n", i, r1[i]);
               }
               
               printf("Command received from master!\r\n");
               switch (r1[EFP_CMD_REGISTER_BYTE +2])
               {
                  case EFP_CMD_PING:
                    printf("Ping!\r\n");
                    r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
                    //efp_set_ack(&slave_efp, EFP_ACK_OK);
                 break;
                 case EFP_CMD_ORDER:
                    printf("Work order\r\n");
        
                    if (slave_efp.mode != EFP_MODE_IDLE)
                    {
                        printf("Cannot accept work, not in idle mode.\r\n");
                        r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_ERR;
                        //efp_set_ack(&slave, EFP_ACK_ERR);
                    } 
                    else
                    {
        
                        //Grab the work value.
                        uint8_t work_value = r1[EFP_CMD_REGISTER_DATA_BYTE + 2];
                        printf("Requested work value is: %u\r\n", work_value);
                    
                        efp_set_job(&slave_efp, work_value);
                        r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
                    }
        
                break;
                case EFP_CMD_STATUS:
                    printf("Check status\r\n");
                    //efp_set_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE, slave.current_job_progress);
                    r1[EFP_CMD_REGISTER_DATA_BYTE +1] = slave_efp.current_job_progress;
                    r1[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = EFP_ACK_OK;
                break;
               }

               slave.stop();
               break;
       }
       for(int i = 0; i < 10; i++) input_buffer[i] = 0;    // Clear buffer
   }
}

/*
#include <stdint.h>
#include "I2CSlaveRK.h"
#include "algorithm.h"
#include "efp.h"

static I2CSlave device(Wire, EFP_SLAVE_ADDR, EFP_SLAVE_REGISTERS);

SYSTEM_THREAD(ENABLED);
static Thread *compute_thread;

void setup()
{
    efp_slave_init(&slave);
    Serial.begin(9600);
    device.begin();

    compute_thread = new Thread("compute_thread", compute);
}

void loop()
{
    //If the master hasn't updated the I2C register, do nothing.
    if(! device.getRegisterSet(slave.reg_val))
        return;
    
    efp_slave_parse_registers(device.getRegister(slave.reg_val), &slave, 0x0);
    Serial.printf("Command received from master: ");
    switch (efp_get_register_byte(&slave, EFP_CMD_REGISTER_BYTE))
    {
        case EFP_CMD_PING:
            Serial.printlnf("Ping");
            efp_set_ack(&slave, EFP_ACK_OK);
            device.setRegister(0x0, efp_pack_registers(&slave));
        break;
        

        case EFP_CMD_RESULT:
            Serial.printlnf("Request result");
            if (slave.mode != EFP_MODE_DONE)
            {
                Serial.printlnf("Cannot give results while still computing");
                efp_set_ack(&slave, EFP_ACK_ERR);
            } 
            else
            {
                uint8_t idxRequested = efp_get_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE);
                if (idxRequested > EFP_JOB_FACTOR || idxRequested <= 0)
                {
                    Serial.printlnf("The requested result index is greater than EFP job factor. No buffer overflows here!");
                    efp_set_ack(&slave, EFP_ACK_ERR);
                }
                else
                {
                    efp_set_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE, slave.current_job_results[idxRequested -1]);
                    efp_set_ack(&slave, EFP_ACK_OK);
                }
            }
            device.setRegister(0x0, efp_pack_registers(&slave));
            
        break;
        case EFP_CMD_RESET:
            Serial.printlnf("Reset");
            if (slave.mode != EFP_MODE_DONE)
            {
                Serial.printlnf("Can only reset when done");
                efp_set_ack(&slave, EFP_ACK_ERR);
            }
            else
            {
                efp_set_ack(&slave, EFP_ACK_OK);
            }
            efp_set_idle(&slave);
            device.setRegister(0x0, efp_pack_registers(&slave));

        break;
        default:
            Serial.printlnf("Unknown command byte received: 0x%02x", slave.registers[EFP_CMD_REGISTER_BYTE]);
        break;
    }
    
    //Serial.printlnf("Some number: 0x%08x", efp_pack_registers(&slave));
    
}

void compute()
{
    while (1)
    {
        if (slave.mode != EFP_MODE_WORK)
        {
            //Serial.printlnf("Waiting for work...");
            //delay(50);
            os_thread_yield();
            continue;
        }

        int start = slave.current_job_start_idx * EFP_JOB_FACTOR +1;
        int end = start + (EFP_JOB_FACTOR -1);
        int n = start;
        Serial.printlnf("Computing %i to %i", start, end);
        
        for (slave.current_job_progress=0; n<=end; ++slave.current_job_progress)
        {
            slave.current_job_results[slave.current_job_progress] = get_nth_digit(n++);
            os_thread_yield();
        }

        efp_set_done(&slave);
        Serial.printlnf("Digit computation done.");
        for (uint8_t x=0; x<EFP_JOB_FACTOR; ++x)
            Serial.printf("%i", slave.current_job_results[x]);
        Serial.printf("\n");
        os_thread_yield();

    }
}*/