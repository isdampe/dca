#ifndef I2C_H
#define I2C_H
#include <stdint.h>

typedef enum
{
	I2C_STATUS_OK,
	I2C_STATUS_ERR_INVALID_DEVICE_NAME,
	I2C_STATUS_ERR_OPEN,
	I2C_STATUS_ERR_IOCTL
} I2C_STATUS;

typedef struct
{
	char device[32];
	uint32_t addr;
	uint32_t fh;
} i2c_obj;

I2C_STATUS i2c_init(i2c_obj *obj, const char *device, const uint32_t addr);
uint8_t i2c_read_byte(const i2c_obj *obj, const uint8_t register_num);
const char *i2c_get_status_str(const I2C_STATUS status);

#endif
