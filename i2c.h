#ifndef I2C_H
#define I2C_H
#include <stdint.h>

typedef struct
{
	char device[32];
	uint32_t addr;
	uint32_t fh;
} i2c_obj;

i2c_obj i2c_create(char *device, uint32_t addr);
uint8_t i2c_read_byte(i2c_obj *obj);

#endif
