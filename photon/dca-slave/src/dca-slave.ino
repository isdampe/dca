#include <stdint.h>
#include "I2CSlaveRK.h"
#include "efp.h"

static I2CSlave device(Wire, EFP_SLAVE_ADDR, EFP_SLAVE_REGISTERS);
static efp_slave slave;

void setup()
{
	efp_slave_init(&slave);
	Serial.begin(9600);
	device.begin();
}

void loop()
{
	uint32_t result = 0x0;
	//If the master hasn't updated the I2C register, do nothing.
	if(! device.getRegisterSet(slave.reg_val))
		return;
	
	efp_slave_parse_registers(device.getRegister(slave.reg_val), &slave, 0x0);
	Serial.printf("Command received from master: ");
	switch (slave.registers[EFP_CMD_REGISTER_BYTE])
	{
		case EFP_CMD_PING:
			Serial.printlnf("Ping");
		break;
		case EFP_CMD_ORDER:
			Serial.printlnf("Work order");
		break;
		case EFP_CMD_STATUS:
			Serial.printlnf("Check status");
		break;
		case EFP_CMD_RESULT:
			Serial.printlnf("Request result");
		break;
		case EFP_CMD_CANCEL:
			Serial.printlnf("Cancel");
		break;
		default:
			Serial.printlnf("Unknown command byte received: 0x%02x", slave.registers[EFP_CMD_REGISTER_BYTE]);
		break;
	}
	
	Serial.printlnf("Some number: 0x%08x", efp_pack_registers(&slave));
	
}