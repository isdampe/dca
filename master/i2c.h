#ifndef I2C_H
#define I2C_H
#include <stdint.h>

typedef enum
{
	I2C_STATUS_OK,
	I2C_STATUS_ERR_INVALID_DEVICE_NAME,
	I2C_STATUS_ERR_OPEN,
	I2C_STATUS_ERR_IOCTL,
	I2C_STATUS_ERR_REG_OUT_OF_BOUNDS,
	I2C_STATUS_ERR_READ_REG,
	I2C_STATUS_ERR_WRITE_REG
} I2C_STATUS;

typedef enum
{
	I2C_HW_PHOTON,
	I2C_HW_MBED
} I2C_HW;

typedef struct
{
	char device[32];
	uint32_t addr;
	uint32_t fh;
	uint8_t reg[6];
	I2C_HW hw_type;
} i2c_obj;

I2C_STATUS i2c_init(i2c_obj *obj, const char *device, const uint32_t addr, const I2C_HW hw_type);
I2C_STATUS i2c_read_reg(i2c_obj *obj);
I2C_STATUS i2c_write_reg(i2c_obj *obj);
I2C_STATUS i2c_set_reg_data(i2c_obj *obj, const uint8_t byte_number, const uint8_t val);
void i2c_close(i2c_obj *obj);
const char *i2c_get_status_str(const I2C_STATUS status);

#endif
